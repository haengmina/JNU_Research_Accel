
// 베어메탈 A53 MobileNetV1 통합 데모
// 기능: 비트스트림 로드 -> 가중치/라벨 로드 -> SD BMP -> 추론 -> Top-5
#include "xil_printf.h"
#include "xil_cache.h"
#include "ff.h"            // FatFs
#include "xtime_l.h"       // 타이밍
#include "xfpga.h"         // FPGA 비트스트림 로드
#include <cstdint>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <cstring>
#include "ref_kernels.h"

using namespace std;

// ---------- 타이밍 헬퍼 ----------
static inline u32 ms_from_counts(XTime t0, XTime t1) {
    return (u32)(((t1 - t0) * 1000U) / COUNTS_PER_SECOND);
}
static inline u32 us_from_counts(XTime t0, XTime t1) {
    return (u32)(((t1 - t0) * 1000000U) / COUNTS_PER_SECOND);
}
static inline void print_ms_us(XTime t0, XTime t1) {
    xil_printf("%u ms (%u us)\r\n", ms_from_counts(t0, t1), us_from_counts(t0, t1));
}

// ---------- SD 헬퍼 ----------
static FATFS fatfs;
static FIL   file;

static bool sd_mount() {
    FRESULT rc = f_mount(&fatfs, "0:/", 1);
    if(rc != FR_OK){ xil_printf("f_mount failed: %d\r\n", rc); return false; }
    xil_printf("[SD] Mounted: 0:/\r\n");
    return true;
}

static bool sd_read_all(const char* path, vector<uint8_t>& out){
    FRESULT rc = f_open(&file, path, FA_READ);
    if(rc != FR_OK){ xil_printf("[SD] open %s failed: %d\r\n", path, rc); return false; }
    UINT fsize = f_size(&file);
    out.resize(fsize);
    UINT br=0; rc = f_read(&file, out.data(), fsize, &br); f_close(&file);
    if(rc!=FR_OK || br!=fsize){ xil_printf("[SD] read failed rc=%d\r\n", rc); return false; }
    return true;
}

// ---------- FPGA 비트스트림 로더 ----------
static bool fpga_load_bitstream(const char* path) {
    vector<uint8_t> bitstream;
    xil_printf("[FPGA] Loading bitstream: %s ...\r\n", path);
    if(!sd_read_all(path, bitstream)) return false;

    XTime t0, t1;
    XTime_GetTime(&t0);
    
    // 캐시 일관성을 위해 비트스트림 메모리 플러시
    Xil_DCacheFlushRange((INTPTR)bitstream.data(), bitstream.size());

    // u32 Status = XFpga_PL_BitStream_Load(&XFpgaInstance, (UINTPTR)bitstream.data(), (UINTPTR)bitstream.data(), bitstream.size(), XFPGA_FULL_BITSTREAM);
    
    xil_printf("[FPGA] Success (Mocked API Call)\r\n");
    XTime_GetTime(&t1);
    xil_printf("[FPGA] Programming time: "); print_ms_us(t0, t1);
    
    return true;
}

// ---------- 가중치 관리 시스템 ----------
struct LayerWeights {
    int layer_id;
    int bits;
    size_t offset;
    size_t size;
    uint8_t* data;
};

static vector<uint8_t> weights_all;
static vector<LayerWeights> layers;

bool load_mobilenet_weights(const char* weights_path, const char* metadata_path) {
    vector<uint8_t> meta_buf;
    if(!sd_read_all(metadata_path, meta_buf)) return false;
    if(!sd_read_all(weights_path, weights_all)) return false;

    int32_t* meta = (int32_t*)meta_buf.data();
    int num_layers = meta_buf.size() / (sizeof(int32_t) * 4);
    
    layers.clear();
    for(int i=0; i<num_layers; ++i) {
        LayerWeights lw;
        lw.layer_id = meta[i*4 + 0];
        lw.bits     = meta[i*4 + 1];
        lw.offset   = (size_t)meta[i*4 + 2];
        lw.size     = (size_t)meta[i*4 + 3];
        lw.data     = weights_all.data() + lw.offset;
        layers.push_back(lw);
    }
    
    xil_printf("[Model] Loaded %d layers from %s\r\n", num_layers, weights_path);
    return true;
}

// ---------- 라벨 & Top-5 ----------
static vector<string> load_labels(const char* path){
    vector<string> labels; vector<uint8_t> buf;
    if(!sd_read_all(path, buf)) return labels;
    string s((char*)buf.data(), (char*)buf.data()+buf.size());
    size_t pos=0;
    while(pos<s.size()){
        size_t nl=s.find('\n',pos); if(nl==string::npos) nl=s.size();
        string line=s.substr(pos,nl-pos); if(!line.empty() && line.back()=='\r') line.pop_back();
        if(!line.empty()) labels.push_back(line);
        pos=(nl==s.size()? nl : nl+1);
    }
    return labels;
}

static void print_top5(const vector<float>& probs, const vector<string>& labels) {
    struct Score { int idx; float val; };
    vector<Score> s; s.reserve(probs.size());
    for (size_t i = 0; i < probs.size(); ++i) s.push_back({(int)i, probs[i]});
    sort(s.begin(), s.end(), [](const Score& a, const Score& b){ return a.val > b.val; });

    int k = min(5, (int)s.size());
    xil_printf("\r\n--- Prediction: Top-%d ---\r\n", k);
    for (int i = 0; i < k; ++i) {
        const char* name = (s[i].idx < (int)labels.size() ? labels[s[i].idx].c_str() : "unknown");
        u32 pct = (u32)lrintf(s[i].val * 100.0f);
        xil_printf(" #%d: %s (%u%%)\r\n", i+1, name, pct);
    }
}

// ---------- BMP 24-bit 로더 ----------
#pragma pack(push,1)
struct BMPFileHeader{ uint16_t bfType; uint32_t bfSize; uint16_t r1; uint16_t r2; uint32_t bfOffBits; };
struct BMPInfoHeader{ uint32_t biSize; int32_t biWidth; int32_t biHeight; uint16_t biPlanes; uint16_t biBitCount; uint32_t biCompression; uint32_t biSizeImage; int32_t biXPelsPerMeter; int32_t biYPelsPerMeter; uint32_t biClrUsed; uint32_t biClrImportant; };
#pragma pack(pop)

static bool load_bmp_24(const char* path, int targetW, int targetH, vector<uint8_t>& out_rgb) {
    FRESULT rc = f_open(&file, path, FA_READ); if(rc!=FR_OK) return false;
    BMPFileHeader fh; BMPInfoHeader ih; UINT br;
    f_read(&file, &fh, sizeof(fh), &br); f_read(&file, &ih, sizeof(ih), &br);
    if(fh.bfType!=0x4D42 || ih.biBitCount!=24){ f_close(&file); return false; }
    
    int W = ih.biWidth, H = abs(ih.biHeight);
    if(W != targetW || H != targetH){ xil_printf("[BMP] Size mismatch: %dx%d (expected %dx%d)\r\n", W, H, targetW, targetH); f_close(&file); return false; }
    
    out_rgb.resize(W*H*3);
    uint32_t row_stride = ((W*3 + 3) & ~3);
    vector<uint8_t> row(row_stride);
    f_lseek(&file, fh.bfOffBits);
    for(int y=0; y<H; ++y){
        f_read(&file, row.data(), row_stride, &br);
        int dst_y = (ih.biHeight > 0) ? (H-1-y) : y;
        uint8_t* dst = &out_rgb[(dst_y*W)*3];
        for(int x=0; x<W; ++x){
            dst[x*3+0]=row[x*3+2]; dst[x*3+1]=row[x*3+1]; dst[x*3+2]=row[x*3+0];
        }
    }
    f_close(&file);
    return true;
}

// ---------- 메인 루틴 ----------
int main() {
    xil_printf("\r\n=== ZCU104 MobileNetV1 Week 3 Integration ===\r\n");

    if(!sd_mount()) return -1;

    // 1. FPGA 비트스트림 로드 (PL 가속기 준비)
    fpga_load_bitstream("0:/mobilenet_wrapper.bit");

    // 2. 가중치 및 라벨 로드
    load_mobilenet_weights("0:/assets/mixed/weights_all.bin", "0:/assets/mixed/metadata.bin");
    vector<string> labels = load_labels("0:/assets/labels.txt");

    // 3. 이미지 로드 (MobileNetV1 표준 224x224 또는 32x32 폴백)
    int W=224, H=224, C=3;
    vector<uint8_t> rgb;
    if(!load_bmp_24("0:/assets/samples/digit_1.bmp", 224, 224, rgb)) {
        if(load_bmp_24("0:/assets/samples/digit_1.bmp", 32, 32, rgb)) {
            W=32; H=32; xil_printf("[Info] Using 32x32 sample\r\n");
        } else {
            xil_printf("[Error] Image load failed\r\n"); return -1;
        }
    }

    // 4. 추론 준비
    const int Cout = 1000;
    const float in_scale=0.02f, w_scale=0.002f, out_scale=0.02f, sm_scale=1.0f/255.0f;
    const int in_zp=128, w_zp=128, out_zp=128, sm_zp=0;

    vector<uint8_t> in_u8(rgb.size());
    memcpy(in_u8.data(), rgb.data(), rgb.size());

    vector<uint8_t> dw_out(rgb.size());
    vector<uint8_t> pw_out(W * H * Cout);
    vector<uint8_t> sm_out(Cout);

    tensor_u8_nhwc_t tin  = {H, W, C,    in_u8.data(),  in_scale, in_zp};
    tensor_u8_nhwc_t tdw  = {H, W, C,    dw_out.data(), out_scale, out_zp};
    tensor_u8_nhwc_t tpw  = {H, W, Cout, pw_out.data(), out_scale, out_zp};
    tensor_u8_nhwc_t tsm  = {1, 1, Cout, sm_out.data(), sm_scale, sm_zp};

    // 5. 추론 실행
    XTime t_start, t_end;
    XTime_GetTime(&t_start);

    if(layers.size() >= 2) {
        dwconv3x3_nhwc_mixed(&tin, layers[0].data, NULL, w_scale, w_zp, layers[0].bits, &tdw, 1);
        pwconv1x1_nhwc_mixed(&tdw, layers[1].data, NULL, w_scale, w_zp, layers[1].bits, &tpw);
    }

    avgpool_global_nhwc_u8(&tpw, &tsm);
    softmax_u8(&tsm, &tsm);

    XTime_GetTime(&t_end);
    xil_printf("[Inference] Total time: "); print_ms_us(t_start, t_end);

    // 6. 결과 출력
    vector<float> probs(Cout);
    for(int i=0; i<Cout; ++i) probs[i] = sm_scale * ((int)sm_out[i] - sm_zp);
    print_top5(probs, labels);

    xil_printf("=== Finished ===\r\n");
    while(1);
    return 0;
}



static inline u32 ms_from_counts(XTime t0, XTime t1) {
    u64 diff = t1 - t0;
    return (u32)((diff * 1000U) / COUNTS_PER_SECOND);
}
static inline u32 us_from_counts(XTime t0, XTime t1) {
    u64 diff = t1 - t0;
    return (u32)((diff * 1000000U) / COUNTS_PER_SECOND);
}
static inline void print_ms_us(XTime t0, XTime t1) {
    xil_printf("%u ms (%u us)\r\n", ms_from_counts(t0, t1), us_from_counts(t0, t1));
}


// ---------- SD Helper ----------
static FATFS fatfs;
static FIL   file;

static bool sd_mount() {
    FRESULT rc = f_mount(&fatfs, "0:/", 1);
    if(rc != FR_OK){ xil_printf("f_mount failed: %d\r\n", rc); return false; }
    xil_printf("SD mounted: 0:/\r\n");
    return true;
}
static bool sd_read_all(const char* path, vector<uint8_t>& out){
    FRESULT rc = f_open(&file, path, FA_READ);
    if(rc != FR_OK){ xil_printf("f_open %s failed: %d\r\n", path, rc); return false; }
    UINT fsize = f_size(&file);
    xil_printf("File %s size: %u bytes\r\n", path, fsize);
    out.resize(fsize);
    UINT br=0; rc = f_read(&file, out.data(), fsize, &br); f_close(&file);
    if(rc!=FR_OK || br!=fsize){ xil_printf("f_read failed rc=%d br=%u size=%u\r\n", rc, br, fsize); return false; }
    return true;
}

// ---------- 최소 BMP 로더 (24-bit BI_RGB) ----------
#pragma pack(push,1)
struct BMPFileHeader{ uint16_t bfType; uint32_t bfSize; uint16_t r1; uint16_t r2; uint32_t bfOffBits; };
struct BMPInfoHeader{ uint32_t biSize; int32_t biWidth; int32_t biHeight; uint16_t biPlanes; uint16_t biBitCount; uint32_t biCompression; uint32_t biSizeImage; int32_t biXPelsPerMeter; int32_t biYPelsPerMeter; uint32_t biClrUsed; uint32_t biClrImportant; };
#pragma pack(pop)

static bool load_bmp_24_stream(const char* path, int& W, int& H, vector<uint8_t>& rgb){
    FRESULT rc = f_open(&file, path, FA_READ); if(rc!=FR_OK){ xil_printf("open %s failed\r\n", path); return false; }
    BMPFileHeader fh; BMPInfoHeader ih; UINT br=0;
    rc = f_read(&file, &fh, sizeof(fh), &br); if(rc!=FR_OK||br!=sizeof(fh)){ xil_printf("read FH failed\r\n"); f_close(&file); return false; }
    rc = f_read(&file, &ih, sizeof(ih), &br); if(rc!=FR_OK||br!=sizeof(ih)){ xil_printf("read IH failed\r\n"); f_close(&file); return false; }
    if(fh.bfType!=0x4D42 || ih.biSize!=40 || ih.biPlanes!=1 || ih.biBitCount!=24 || ih.biCompression!=0){ xil_printf("unsupported BMP\r\n"); f_close(&file); return false; }
    W = ih.biWidth; H = (ih.biHeight>0? ih.biHeight : -ih.biHeight); bool bottom_up = (ih.biHeight>0);
    uint32_t row_stride = ((W*3 + 3) & ~3);
    rc = f_lseek(&file, fh.bfOffBits); if(rc!=FR_OK){ xil_printf("seek pixels failed\r\n"); f_close(&file); return false; }
    rgb.resize(W*H*3);
    vector<uint8_t> row(row_stride);
    for(int y=0;y<H;++y){
        UINT rb=0; rc=f_read(&file,row.data(),row_stride,&rb); if(rc!=FR_OK||rb!=row_stride){ xil_printf("row read failed\r\n"); f_close(&file); return false; }
        int dst_y = bottom_up? (H-1-y) : y;
        uint8_t* dst = &rgb[(dst_y*W)*3];
        for(int x=0;x<W;++x){ // BGR->RGB
            dst[x*3+0]=row[x*3+2]; dst[x*3+1]=row[x*3+1]; dst[x*3+2]=row[x*3+0];
        }
    }
    f_close(&file);
    return true;
}

// ---------- 라벨 (Labels) ----------
static vector<string> load_labels(const char* path){
    vector<string> labels; vector<uint8_t> buf;
    if(!sd_read_all(path, buf)) return labels;
    string s((char*)buf.data(), (char*)buf.data()+buf.size());
    size_t pos=0;
    while(pos<s.size()){
        size_t nl=s.find('\n',pos); if(nl==string::npos) nl=s.size();
        string line=s.substr(pos,nl-pos); if(!line.empty() && line.back()=='\r') line.pop_back();
        if(!line.empty()) labels.push_back(line);
        pos=(nl==s.size()? nl : nl+1);
    }
    return labels;
}

// ---------- 상위-5 (Top-5) ----------
static void print_top5(const std::vector<float>& probs, const std::vector<std::string>& labels) {
    struct Score { int idx; float val; };
    std::vector<Score> s; s.reserve(probs.size());
    for (size_t i = 0; i < probs.size(); ++i) s.push_back({(int)i, probs[i]});
    std::sort(s.begin(), s.end(), [](const Score& a, const Score& b){ return a.val > b.val; });

    int k = std::min(5, (int)s.size());
    xil_printf("Top-%d:\r\n", k);
    for (int i = 0; i < k; ++i) {
        const char* name = (s[i].idx < (int)labels.size() ? labels[s[i].idx].c_str() : "(no-label)");
        u32 score255 = (u32)lrintf(s[i].val * 255.0f);
        u32 pct      = (u32)lrintf(s[i].val * 100.0f);
        xil_printf(" %d) %s : %u/255 (%u%%)\r\n", i+1, name, score255, pct);
    }
}


// ---------- 메인 (Main) ----------
int main(){

    xil_printf("mobilenet_bm: bare-metal demo\r\n");
    if(!sd_mount()) return -1;

    // 필요한 SD 자산:
    //   0:/assets/test_32x32.bmp        (24비트 BMP, 32x32 RGB)
    //   0:/assets/labels.txt            (8줄, 클래스당 한 줄)
    //   0:/assets/dw3x3_c3.bin          (DW 가중치, Cin=3, 채널당 3x3)
    //   0:/assets/pw1x1_c8x3.bin        (PW 가중치, Cout=8, Cin=3)

    int W=0,H=0; vector<uint8_t> rgb;
    if(!load_bmp_24_stream("0:/assets/samples/digit_1.bmp", W, H, rgb)){ xil_printf("BMP load failed\r\n"); return -1; }
    xil_printf("Image 2 loaded: %dx%d\r\n", W, H);
    if(W!=32 || H!=32){ xil_printf("For this demo, please use 32x32 BMP.\r\n"); return -1; }

    // 양자화 파라미터 - 스케일
    const float in_scale = 0.02f;  const int in_zp = 128;
    const float w_scale  = 0.0019579321f;  const int w_zp = 128;
    const float out_scale = 0.02f; const int out_zp = 128;
    const float sm_scale = 1.0f/255.0f; const int sm_zp = 0;
//    const float in_scale=0.02f; const int in_zp=128;
//    const float w_scale=0.02f;  const int w_zp=128;
//    const float out_scale=0.02f; const int out_zp=128;
//    const float sm_scale=1.0f/255.0f; const int sm_zp=0;

    // NHWC 버퍼 - 클래스 수
    const int Cin=3; const int Cout=10;
    vector<uint8_t> in_u8(W*H*Cin), dw_out(W*H*Cin), pw_out(W*H*Cout), sm_u8(Cout);

    // 전처리: RGB -> NHWC uint8 (3 채널)
    for(int y=0;y<H;++y){
        for(int x=0;x<W;++x){
            uint8_t r=rgb[(y*W+x)*3+0], g=rgb[(y*W+x)*3+1], b=rgb[(y*W+x)*3+2];
            in_u8[(y*W+x)*Cin+0]=r; in_u8[(y*W+x)*Cin+1]=g; in_u8[(y*W+x)*Cin+2]=b;
        }
    }

    // 가중치 및 메타데이터 로드 (혼합 정밀도)
    vector<uint8_t> dw_w, pw_w, meta_buf;
    if(!sd_read_all("0:/assets/mixed/dw_mixed.bin", dw_w)) return -1;
    if(!sd_read_all("0:/assets/mixed/pw_mixed.bin", pw_w)) return -1;
    if(!sd_read_all("0:/assets/mixed/metadata.bin", meta_buf)) return -1;

    int32_t* meta = (int32_t*)meta_buf.data();
    int dw_bits = meta[1]; // [ID, Bits, Offset, Size]
    int pw_bits = meta[5]; // Next layer starts at index 4

    xil_printf("Mixed-Precision Config Loaded:\r\n");
    xil_printf("  DW: %d-bit, PW: %d-bit\r\n", dw_bits, pw_bits);

    // 라벨
    vector<string> labels = load_labels("0:/assets/labels.txt");
    if(labels.size()!= (size_t)Cout){ xil_printf("labels count != Cout (%d)\r\n", Cout); }

    // 텐서 (Tensors)
    tensor_u8_nhwc_t tin={H,W,Cin,in_u8.data(), in_scale, in_zp};
    tensor_u8_nhwc_t tdw={H,W,Cin,dw_out.data(), out_scale, out_zp};
    tensor_u8_nhwc_t tpw={H,W,Cout,pw_out.data(), out_scale, out_zp};
    tensor_u8_nhwc_t tsm_in={1,1,Cout,sm_u8.data(), sm_scale, sm_zp};
    tensor_u8_nhwc_t tsm_out={1,1,Cout,sm_u8.data(), sm_scale, sm_zp};

    // --- (선택적이지만 권장됨) 참조 avgpool 사용 ---
    tensor_u8_nhwc_t tavg_out = {1,1,Cout, sm_u8.data(), sm_scale, sm_zp};
    avgpool_global_nhwc_u8(&tpw, &tavg_out);

    // 실행 시간 (Runtimes)
    XTime t0,t1;

    // Depthwise (Mixed-Precision Emulator)
    XTime_GetTime(&t0);
    dwconv3x3_nhwc_mixed(&tin, dw_w.data(), NULL, w_scale, w_zp, dw_bits, &tdw, 1);
    XTime_GetTime(&t1);
    xil_printf("DWConv (Mixed): "); print_ms_us(t0, t1);

    // Pointwise (Mixed-Precision Emulator)
    XTime_GetTime(&t0);
    pwconv1x1_nhwc_mixed(&tdw, pw_w.data(), NULL, w_scale, w_zp, pw_bits, &tpw);
    XTime_GetTime(&t1);
    xil_printf("PWConv (Mixed): "); print_ms_us(t0, t1);


    // Global average pool -> softmax 입력 (sm_u8)
    XTime_GetTime(&t0);
    for(int c=0;c<Cout;++c){
        float sum=0.0f;
        for(int y=0;y<H;++y) for(int x=0;x<W;++x){
            uint8_t q=pw_out[(y*W+x)*Cout+c];
            sum += (out_scale*((int)q - out_zp));
        }
        float mean=sum/(float)(H*W);
        sm_u8[c] = (uint8_t)((int)lrintf(mean/sm_scale));
    }
    XTime_GetTime(&t1);
    xil_printf("AvgPool: "); print_ms_us(t0, t1);

    // Softmax
    XTime_GetTime(&t0);
    softmax_u8(&tsm_in, &tsm_out);
    XTime_GetTime(&t1);
    xil_printf("Softmax: "); print_ms_us(t0, t1);

    // float로 변환 및 Top-5 출력
    vector<float> probs(Cout);
    for(int c=0;c<Cout;++c) probs[c] = sm_scale * ((int)sm_u8[c] - sm_zp);
    print_top5(probs, labels);

    // RAM 사용량 (할당된 버퍼)
    xil_printf("RAM bytes: in=%lu dw_out=%lu pw_out=%lu sm=%lu\r\n",
        (unsigned long)in_u8.size(), (unsigned long)dw_out.size(),
        (unsigned long)pw_out.size(), (unsigned long)sm_u8.size());

    xil_printf("Done.\r\n");


    while(1){}
    return 0;
}

