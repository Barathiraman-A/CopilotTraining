# Azure VM Infrastructure

This folder contains Bicep infrastructure-as-code files to deploy a basic Azure Virtual Machine.

## What Gets Deployed

- **Virtual Machine**: Ubuntu 22.04 LTS (Standard_B2s by default)
- **Virtual Network**: 10.0.0.0/16 with default subnet 10.0.1.0/24
- **Network Security Group**: Allows SSH (port 22) inbound traffic
- **Public IP Address**: Static IP with DNS name
- **Network Interface**: Connects VM to VNet and public IP
- **Managed Identity**: System-assigned identity enabled for Azure resource access

## Prerequisites

1. **Azure CLI** installed and logged in:
   ```powershell
   az login
   ```

2. **SSH Key Pair**: Generate if you don't have one:
   ```powershell
   ssh-keygen -t rsa -b 4096 -f ~\.ssh\azure_vm_key
   ```

## Deployment Steps

### 1. Update Parameters

Edit `main.bicepparam` and replace the `sshPublicKey` value with your actual SSH public key:
```powershell
Get-Content ~\.ssh\azure_vm_key.pub
```

### 2. Create Resource Group

```powershell
az group create --name rg-vm-demo --location eastus
```

### 3. Preview Deployment (What-If)

```powershell
az deployment group what-if `
  --resource-group rg-vm-demo `
  --template-file infra/main.bicep `
  --parameters infra/main.bicepparam
```

### 4. Deploy

```powershell
az deployment group create `
  --resource-group rg-vm-demo `
  --template-file infra/main.bicep `
  --parameters infra/main.bicepparam `
  --name vm-deployment
```

### 5. Get Connection Info

```powershell
az deployment group show `
  --resource-group rg-vm-demo `
  --name vm-deployment `
  --query properties.outputs
```

## Connect to VM

After deployment, use the SSH command from outputs:
```powershell
ssh azureuser@<your-vm-fqdn>
```

Or use the private key explicitly:
```powershell
ssh -i ~\.ssh\azure_vm_key azureuser@<your-vm-fqdn>
```

## Cost Estimation

**Standard_B2s** in East US (as of 2025):
- ~$30-40/month (pay-as-you-go)
- ~$20-25/month (1-year reserved instance)

**Storage**:
- Premium SSD OS disk (128GB default): ~$20/month

## Security Features

- ✅ Password authentication disabled (SSH key only)
- ✅ Network Security Group with minimal rules
- ✅ System-assigned Managed Identity
- ✅ Automatic OS patching enabled
- ✅ Boot diagnostics enabled
- ✅ Latest Ubuntu LTS image

## Cleanup

Delete the entire resource group:
```powershell
az group delete --name rg-vm-demo --yes --no-wait
```

## Customization

### Change VM Size
Edit `main.bicepparam` and update `vmSize`:
- `Standard_B1s`: 1 vCPU, 1GB RAM (~$10/month)
- `Standard_B2s`: 2 vCPUs, 4GB RAM (~$35/month)
- `Standard_D2s_v3`: 2 vCPUs, 8GB RAM (~$70/month)

### Change Location
Update the resource group creation command:
```powershell
az group create --name rg-vm-demo --location westus2
```

### Add Data Disk
Add to the VM's `storageProfile` in `main.bicep`:
```bicep
dataDisks: [
  {
    name: '${vmName}-datadisk-01'
    diskSizeGB: 128
    lun: 0
    createOption: 'Empty'
    managedDisk: {
      storageAccountType: 'Premium_LRS'
    }
  }
]
```
