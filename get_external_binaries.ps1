############################################
# DXC
############################################
$Url = 'https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.8.2403.2/dxc_2024_03_29.zip' 
$ZipFile = $PSScriptRoot + '/' + $(Split-Path -Path $Url -Leaf)

$ExtractPath = $PSScriptRoot + '/external/dxc/'
New-Item -ItemType Directory -Path $ExtractPath -Force
 
Invoke-WebRequest -Uri $Url -OutFile $ZipFile 

Expand-Archive $ZipFile -DestinationPath $ExtractPath -Force
Remove-Item -Path $ZipFile -Force -Confirm:$false

############################################
# Sharpmake
############################################
$Url = 'https://github.com/ubisoft/Sharpmake/releases/download/0.18.2/Sharpmake-0.18.2-net5.0-Windows.zip' 
$ZipFile = $PSScriptRoot + '/' + $(Split-Path -Path $Url -Leaf)

$ExtractPath = $PSScriptRoot + '/external/sharpmake/'
New-Item -ItemType Directory -Path $ExtractPath -Force
 
Invoke-WebRequest -Uri $Url -OutFile $ZipFile

Expand-Archive $ZipFile -DestinationPath $ExtractPath -Force
Remove-Item -Path $ZipFile -Force -Confirm:$false

############################################
# DX12 Agility SDK
############################################
$Url = 'https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/1.711.3-preview' 
$ZipFile = $PSScriptRoot + '/agility.zip'

$ExtractPath = $PSScriptRoot + '/external/DX12_Agility_SDK/'
New-Item -ItemType Directory -Path $ExtractPath -Force

Invoke-WebRequest -Uri $Url -OutFile $ZipFile

Expand-Archive $ZipFile -DestinationPath $ExtractPath -Force
Remove-Item -Path $ZipFile -Force -Confirm:$false

############################################
# WinPixEventRuntime
############################################
$Url = 'https://www.nuget.org/api/v2/package/WinPixEventRuntime/1.0.240308001' 
$ZipFile = $PSScriptRoot + '/pix.zip'

$ExtractPath = $PSScriptRoot + '/external/WinPixEventRuntime/'
New-Item -ItemType Directory -Path $ExtractPath -Force

Invoke-WebRequest -Uri $Url -OutFile $ZipFile

Expand-Archive $ZipFile -DestinationPath $ExtractPath -Force
Remove-Item -Path $ZipFile -Force -Confirm:$false