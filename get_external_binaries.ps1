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
# WinPixEventRuntime
############################################
$Url = 'https://www.nuget.org/api/v2/package/WinPixEventRuntime/1.0.240308001' 
$ZipFile = $PSScriptRoot + '/pix.zip'

$ExtractPath = $PSScriptRoot + '/external/WinPixEventRuntime/'
New-Item -ItemType Directory -Path $ExtractPath -Force

Invoke-WebRequest -Uri $Url -OutFile $ZipFile

Expand-Archive $ZipFile -DestinationPath $ExtractPath -Force
Remove-Item -Path $ZipFile -Force -Confirm:$false