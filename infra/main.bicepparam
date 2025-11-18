using './main.bicep'

// Parameters for VM deployment
// Update these values according to your requirements

param vmName = 'vm-demo'
param vmSize = 'Standard_B2s'  // 2 vCPUs, 4GB RAM - suitable for basic workloads
param adminUsername = 'azureuser'

// TODO: Replace with your actual SSH public key
// Generate one with: ssh-keygen -t rsa -b 4096
param sshPublicKey = 'ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAACAQC... your-ssh-public-key-here'
