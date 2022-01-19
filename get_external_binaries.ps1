############################################
# DXC
############################################
$Url = 'https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.6.2112/dxc_2021_12_08.zip' 
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


