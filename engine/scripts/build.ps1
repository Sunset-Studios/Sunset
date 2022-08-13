param(
    [switch]$development=$false,
    [switch]$headless=$false,
    [switch]$clean=$false
)

$BinPath = "$PSScriptRoot/../bin";

if (-Not (Test-Path $BinPath -PathType Container))
{
    mkdir $BinPath;
}

if ($clean)
{
    rm -r -force $BinPath; mkdir $BinPath;
}

pushd; cd $BinPath;

$exec = 'cmake -G "Visual Studio 17 2022"'

if ($development)
{
    $exec = "$exec -DDEVELOPMENT=ON"
}

if ($headless)
{
    $exec = "$exec -DHEADLESS=ON"
}

Invoke-Expression "$exec ../..;"

if (Get-Command "devenv.com" -ErrorAction SilentlyContinue)
{
    Invoke-Expression "devenv.com sunset.sln /build"
}

popd;