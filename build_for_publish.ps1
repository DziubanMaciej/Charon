# Pre-cleanup
rm Charon.zip -ErrorAction Ignore

$version = Read-Host -Prompt "Input release version: "
$buildDir = "build"
$binaryDir = "$buildDir\bin\Release"

# Run CMake and compile
mkdir $buildDir -Force | out-null
cmake -S . -B $buildDir
cmake --build $buildDir --config Release

# Run tests

# Pack
echo $version > $binaryDir\version.txt
$compress = @{
    Path = "$binaryDir\Charon.exe", "$binaryDir\CharonDaemon.exe", "$binaryDir\version.txt"
    DestinationPath = "Charon.zip"
}
Compress-Archive @compress -Update
