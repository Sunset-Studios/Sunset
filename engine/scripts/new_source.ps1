param(
    $Name,
    $BasePath = "engine/src"
)

while ($null -eq $Name -or $Name -eq '')
{
    $Name = read-host -Prompt "Enter a path from src for the new source file`n- (Ending the file path with a file extension will generate the single file)`n- (Ending the file without a file extension will generate a .h/.cpp wombo combo for the given file path)`n" 
}

$SrcPath = "$PSScriptRoot/../../$BasePath"

Push-Location; Set-Location $SrcPath;

# Only create the single item if a file extension was provided
if ($Name -like "*`.*")
{
    New-Item -Path $SrcPath/$Name -ItemType file -Force
}
# Otherwise auto create a .cpp/.h wombo combo
else
{
    $H = $SrcPath + "/" + $Name + ".h";
    $CPP = $SrcPath + "/" + $Name + ".cpp";
    $HContent = "#pragma once`n`nnamespace Sunset`n{`n`n}`n";
    $CPPContent = '#include "' + $Name + ".h" + '"' + "`n`nnamespace Sunset`n{`n`n}`n";
    New-Item -Path $H -ItemType File -Value $HContent -Force
    New-Item -Path $CPP -ItemType File -Value $CPPContent -Force
}

Pop-Location;

cmd.exe /c "$PSScriptRoot/../../build.bat"