# FluffOS Docker Setup Script
# This script provides deterministic setup for FluffOS development environment
# Date: 2025-07-11

param(
    [string]$ReportsDir = "reports",
    [switch]$Force = $false
)

# Function to write log messages
function Write-Log {
    param([string]$Message, [string]$Level = "INFO")
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    Write-Host "[$timestamp] [$Level] $Message"
}

# Get current date for workspace
$dateString = Get-Date -Format "yyyyMMdd"
$workspaceDir = "$ReportsDir/$dateString"

Write-Log "Starting FluffOS Docker Setup" "INFO"
Write-Log "Workspace: $workspaceDir" "INFO"

# Ensure we're in the FluffOS directory
if (-not (Test-Path "Dockerfile.ubuntu")) {
    Write-Log "ERROR: Dockerfile.ubuntu not found. Please run this script from the FluffOS root directory." "ERROR"
    exit 1
}

# Create workspace directory
if (-not (Test-Path $workspaceDir)) {
    New-Item -Path $workspaceDir -ItemType Directory -Force | Out-Null
    Write-Log "Created workspace directory: $workspaceDir" "INFO"
}

# Check if repository is clean
$gitStatus = git status --porcelain
if ($gitStatus) {
    Write-Log "WARNING: Repository has uncommitted changes" "WARN"
    if (-not $Force) {
        Write-Log "Use -Force to continue anyway" "WARN"
        exit 1
    }
}

# Get current commit info
$commitHash = git rev-parse HEAD
$commitMessage = git log -1 --pretty=format:"%s"
Write-Log "Current commit: $commitHash" "INFO"
Write-Log "Commit message: $commitMessage" "INFO"

# Build Docker image
Write-Log "Building FluffOS Docker image..." "INFO"
$buildResult = docker build -f Dockerfile.ubuntu -t fluffos-ubuntu . 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Log "ERROR: Docker build failed" "ERROR"
    Write-Log $buildResult "ERROR"
    exit 1
}
Write-Log "Docker image built successfully" "INFO"

# Verify FluffOS binaries
Write-Log "Verifying FluffOS binaries..." "INFO"
$binariesResult = docker run --rm --entrypoint=/bin/bash fluffos-ubuntu -c "ls -la bin/ | grep -E '^-.*driver|lpcc|json2o|o2json|portbind|symbol'"
Write-Log "FluffOS binaries verified:" "INFO"
Write-Log $binariesResult "INFO"

# Create development container with all tools for verification
Write-Log "Verifying build prerequisites..." "INFO"
$prereqResult = docker run --rm --entrypoint=/bin/bash ubuntu:22.04 -c @"
apt-get update >/dev/null 2>&1 && 
DEBIAN_FRONTEND=noninteractive apt-get install -y build-essential git bison cmake libmysqlclient-dev libpcre3-dev libpq-dev libsqlite3-dev libssl-dev libz-dev libjemalloc-dev libicu-dev cppcheck clang ripgrep >/dev/null 2>&1 && 
echo 'CMake:' && cmake --version | head -n1 && 
echo 'Python3:' && python3 --version && 
echo 'GCC:' && gcc --version | head -n1 && 
echo 'Clang:' && clang --version | head -n1 && 
echo 'Cppcheck:' && cppcheck --version && 
echo 'Ripgrep:' && rg --version | head -n1 && 
echo 'All prerequisites verified!'
"@

if ($LASTEXITCODE -eq 0) {
    Write-Log "All prerequisites verified successfully" "INFO"
    Write-Log $prereqResult "INFO"
} else {
    Write-Log "ERROR: Prerequisites verification failed" "ERROR"
    exit 1
}

# Save setup information
$setupInfo = @{
    Date = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    CommitHash = $commitHash
    CommitMessage = $commitMessage
    DockerImage = "fluffos-ubuntu:latest"
    WorkspaceDir = $workspaceDir
    Status = "COMPLETE"
} | ConvertTo-Json -Depth 2

$setupInfo | Out-File -FilePath "$workspaceDir/setup_info.json" -Encoding UTF8

Write-Log "Setup information saved to: $workspaceDir/setup_info.json" "INFO"
Write-Log "FluffOS Docker setup completed successfully!" "INFO"
Write-Log "Docker image: fluffos-ubuntu" "INFO"
Write-Log "Workspace: $workspaceDir" "INFO"

# Display usage instructions
Write-Log "Usage Instructions:" "INFO"
Write-Log "Run FluffOS container: docker run --rm -it fluffos-ubuntu /bin/bash" "INFO"
Write-Log "Run with config file: docker run --rm -v 'C:\path\to\config:/config' fluffos-ubuntu /config/config.file" "INFO"
Write-Log "Development container: docker run --rm -it --entrypoint=/bin/bash ubuntu:22.04" "INFO"
