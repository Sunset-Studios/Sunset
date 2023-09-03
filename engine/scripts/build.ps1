param(
    [switch]$development=$false,
    [switch]$headless=$false,
    [switch]$clean=$false,
    $name
)

$ProjectConfigPath = "$PSScriptRoot/../../project_config.json"
$BinPath = "$PSScriptRoot/../bin"
$ConfigContent = @{ 'project_name' = '' }

if (-Not (Test-Path $ProjectConfigPath -PathType Leaf))
{
    while ($null -eq $name -or $name -eq '')
    {
        $name = read-host -Prompt "Enter a project name for you new project`n" 
    }
}
else
{
    $ConfigContent = Get-Content $ProjectConfigPath | ConvertFrom-Json
    if ($ConfigContent.PSObject.Properties.Name -contains 'project_name')
    {
        $name = $ConfigContent.project_name
    }
    else
    {
        while ($null -eq $name -or $name -eq '')
        {
            $name = read-host -Prompt "Enter a project name for you new project`n" 
        }
    }
}

$ConfigContent.project_name = $name
$ConfigContent | ConvertTo-Json -Depth 10 | Set-Content $ProjectConfigPath 

$ProjectPath = "$PSScriptRoot/../../$name"

if (-Not (Test-Path $ProjectPath -PathType Container))
{
    mkdir $ProjectPath
    New-Item -Path "$ProjectPath/CMakeLists.txt"
}

if (-Not (Test-Path $BinPath -PathType Container))
{
    mkdir $BinPath
}

if ($clean)
{
    Remove-Item -r -force $BinPath
    mkdir $BinPath
}

Push-Location
Set-Location $BinPath

$exec = "git submodule update --init --recursive"
$exec = "$exec; cmake -G `"Visual Studio 17 2022`" -DAPPLICATION_NAME=$name"

if ($development)
{
    $exec = "$exec -DDEVELOPMENT=ON"
}

if ($headless)
{
    $exec = "$exec -DHEADLESS=ON"
}

Invoke-Expression "$exec ../.."

if (Get-Command "devenv.com" -ErrorAction SilentlyContinue)
{
    Invoke-Expression "devenv.com sunset.sln /build"
}

Pop-Location