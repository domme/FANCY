$Url = 'https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.6.2112/dxc_2021_12_08.zip' 
$ZipFile = $PSScriptRoot + '/' + $(Split-Path -Path $Url -Leaf)

$ExtractPath = $PSScriptRoot + '/external/dxc/'
New-Item -ItemType Directory -Path $ExtractPath
 
Invoke-WebRequest -Uri $Url -OutFile $ZipFile 

Expand-Archive $ZipFile -DestinationPath $ExtractPath
Remove-Item -Path $ZipFile