$env_data = Get-Content .env
$GLSLC_PATH = $env_data[4].Substring($file_data.Length + 8)
$VCPKG_TOOLCHAIN_FILE = $env_data[5].Substring($file_data.Length + 12) + "\scripts\buildsystems\vcpkg.cmake"

mkdir "Visual Studio"
cmake -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE="$VCPKG_TOOLCHAIN_FILE" -DVCPKG_TARGET_TRIPLET="x64-windows" -B ./"Visual Studio"

Set-Location shaders
$OLD_SPV_FILES = Get-ChildItem -Name -Include *.spv
foreach($OLD_SPV_FILE in $OLD_SPV_FILES){ & Remove-Item $OLD_SPV_FILE}

$SHADER_FILES = Get-ChildItem -Name -Exclude *.sh
foreach($SHADER_FILE in $SHADER_FILES)
{
	& $GLSLC_PATH -c $SHADER_FILE
}
Set-Location ..
