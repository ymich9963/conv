$conv_install_path = 'C:\Program Files\conv'
$conv_exe_link = "https://github.com/ymich9963/conv/releases/latest/download/conv.exe"

if (Test-Path -Path $conv_install_path) {
    Write-Output "Removing previously installed executable."
    Remove-Item $conv_install_path -r # rm command
}

New-Item -Path $conv_install_path -ItemType Directory | Out-Null # make new dir and suppress output
curl -fsSLO $conv_exe_link
Move-Item conv.exe $conv_install_path # mv command
Write-Output "Downloaded executable." # echo command

$Sys_Env_Path_Value = Get-ItemProperty -Path 'HKLM:\SYSTEM\CurrentControlSet\Control\Session Manager\Environment\' -Name Path 

# Change the backslashes to frontslashes so that -split can work
$conv_install_path_frontslash = $conv_install_path -replace "\\","/"
$Sys_Env_Path_Value_frontslash = $Sys_Env_Path_Value.Path -replace "\\", "/"

# Check if the install path exists by splitting the Path variable value
$conv_path_check = $Sys_Env_Path_Value_frontslash -split $conv_install_path_frontslash | Measure-Object 

if ($conv_path_check.Count -igt 1) {
    Write-Output "Detected previous conv installation."
    Write-Output "Nothing was added to the system Path variable."
} else {
    Write-Output "Detected no previous conv install."
    Write-Output "Adding executable to system Path environment variable."
    $New_Path_Value = $Sys_Env_Path_Value.Path + ";" + $conv_install_path + ";" 
    Set-ItemProperty -Path 'HKLM:\SYSTEM\CurrentControlSet\Control\Session Manager\Environment\' -Name Path -Value $New_Path_Value # set the system environment variable for conv 
}

Write-Output "Succesfully installed conv."

Read-Host -Prompt "Press Enter to exit"

Exit

