
import torch
import torch.nn as nn
import torch.nn.functional as F
import numpy as np
import json
import os
from synth_digits_train import TinyDigitDSCNN, SyntheticDigits

def get_hessian_max_eigenvalue(model, criterion, data, device, n_iter=10):
    """
    Computes the maximum eigenvalue of the Hessian for each parameter group 
    using the Power Iteration method.
    """
    model.eval()
    inputs, targets = data
    inputs, targets = inputs.to(device), targets.to(device)
    
    outputs = model(inputs)
    loss = criterion(outputs, targets)
    
    params = [p for p in model.parameters() if p.requires_grad]
    grads = torch.autograd.grad(loss, params, create_graph=True)
    
    # Flatten grads
    grads_flatten = torch.cat([g.reshape(-1) for g in grads])
    
    # Initial random vector
    v = [torch.randn_like(p).to(device) for p in params]
    v_flatten = torch.cat([vi.reshape(-1) for vi in v])
    v_flatten /= torch.norm(v_flatten)
    
    for _ in range(n_iter):
        # Hessian-vector product using double backprop
        # H*v = grad(grad(loss) * v)
        gv = torch.sum(grads_flatten * v_flatten)
        hv = torch.autograd.grad(gv, params, retain_graph=True)
        hv_flatten = torch.cat([hvi.reshape(-1) for hvi in hv])
        
        # Power iteration step
        eigenvalue = torch.dot(v_flatten, hv_flatten)
        v_flatten = hv_flatten / (torch.norm(hv_flatten) + 1e-8)
        
    return eigenvalue.item()

def analyze_sensitivity(model_path, device):
    state = torch.load(model_path, map_location=device)
    model = TinyDigitDSCNN(cout=10).to(device)
    model.load_state_dict(state)
    
    dataset = SyntheticDigits(n_per_class=10, seed=999) # Small subset for analysis
    loader = torch.utils.data.DataLoader(dataset, batch_size=100, shuffle=False)
    data = next(iter(loader))
    
    criterion = nn.CrossEntropyLoss()
    
    sensitivities = {}
    
    # Layer-wise analysis (simplified for this model)
    # In a full MobileNet, we would do this per layer or per channel.
    # Here we have 'dw' and 'pw'.
    
    for name, module in model.named_modules():
        if isinstance(module, nn.Conv2d):
            print(f"Analyzing layer: {name}")
            # Note: For real per-channel, we'd need to mask other channels
            # or use a more granular approach. For now, let's get layer-wise.
            
            # Simple approximation: take the max eigenvalue of the layer's parameters
            # This is a proxy for sensitivity.
            
            # We focus on the parameters of THIS specific module
            params = list(module.parameters())
            if not params: continue
            
            # Power iteration for this specific layer's parameters
            outputs = model(data[0].to(device))
            loss = criterion(outputs, data[1].to(device))
            grads = torch.autograd.grad(loss, params, create_graph=True)
            grads_flat = torch.cat([g.reshape(-1) for g in grads])
            
            v = torch.randn_like(grads_flat).to(device)
            v /= torch.norm(v)
            
            eigenvalue = 0
            for _ in range(10):
                gv = torch.sum(grads_flat * v)
                hv = torch.autograd.grad(gv, params, retain_graph=True)
                hv_flat = torch.cat([hvi.reshape(-1) for hvi in hv])
                eigenvalue = torch.dot(v, hv_flat).item()
                v = hv_flat / (torch.norm(hv_flat) + 1e-8)
                
            sensitivities[name] = abs(eigenvalue)
            print(f"  Max Eigenvalue: {sensitivities[name]:.6f}")

    # Bit Allocation based on ranking
    # 3.5x speedup goal -> 4-bit is 2x faster than 8-bit.
    # We want a mix: 4, 6, 8.
    
    sorted_layers = sorted(sensitivities.items(), key=lambda x: x[1], reverse=True)
    
    allocation = {}
    # Simple rule: Top 33% -> 8bit, Mid 33% -> 6bit, Bottom 34% -> 4bit
    # Since we only have 2 layers here, let's manually or proportional assign.
    # If DW is more sensitive (common), it gets 8-bit.
    
    for i, (name, val) in enumerate(sorted_layers):
        if i == 0:
            allocation[name] = 8
        elif i == 1:
            allocation[name] = 6
        else:
            allocation[name] = 4
            
    return allocation, sensitivities

if __name__ == "__main__":
    device = "cuda" if torch.cuda.is_available() else "cpu"
    model_path = "artifacts/digits_tiny_dsconv.pt"
    
    if not os.path.exists(model_path):
        print(f"Model not found at {model_path}. Training first...")
        from synth_digits_train import train
        train(epochs=2)
        
    allocation, sensitivities = analyze_sensitivity(model_path, device)
    
    result = {
        "sensitivities": sensitivities,
        "allocation": allocation
    }
    
    with open("artifacts/hawq_results.json", "w") as f:
        json.dump(result, f, indent=4)
        
    print("\nHAWQ Analysis Result:")
    print(json.dumps(result, indent=4))
