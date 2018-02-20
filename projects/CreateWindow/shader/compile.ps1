
# Execute following command from PowerShell then run this script file everytime
# Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope Process

# Or, execute this only once and you're good to go
# Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser

# Set to your path
$spirvCompiler = "D:\_ProgramInstall\VulkanSDK\1.0.65.1\Bin\glslangValidator.exe"

# compile shader files with extension .vert or .frag
$shaderFiles = Get-ChildItem -File
$shaderFiles | ForEach-Object {
    If (($_.Extension -eq ".vert") -or ($_.Extension -eq ".frag")) {
        $command = "$spirvCompiler -V $_"
        Invoke-Expression $command
    }
}