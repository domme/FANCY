############################################
# Download WinPixEventRuntime from NuGet
############################################

$Url = 'https://www.nuget.org/api/v2/package/WinPixEventRuntime/1.0.240308001'
$ZipFile = $PSScriptRoot + '\pix.zip'
$ExtractPath = $PSScriptRoot + '\external\WinPixEventRuntime\'

Write-Host "Downloading WinPixEventRuntime from NuGet..." -ForegroundColor Cyan
New-Item -ItemType Directory -Path $ExtractPath -Force | Out-Null

try {
    Invoke-WebRequest -Uri $Url -OutFile $ZipFile -ErrorAction Stop
    Write-Host "Download complete. Extracting..." -ForegroundColor Cyan
    Expand-Archive $ZipFile -DestinationPath $ExtractPath -Force
    Remove-Item -Path $ZipFile -Force
    Write-Host "WinPixEventRuntime installed successfully!" -ForegroundColor Green
}
catch {
    Write-Host "Error downloading WinPixEventRuntime: $_" -ForegroundColor Red
    exit 1
}
