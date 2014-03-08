Imports System
Imports EnvDTE
Imports EnvDTE80
Imports EnvDTE90
Imports System.Security.AccessControl
Imports System.Threading.Thread
Imports System.Diagnostics

Public Module Combined

    Sub DoAnalizeShader(ByVal nvidia As Integer, ByVal amd As Integer, ByVal nvidia_short As Integer, ByVal sm As Integer)

        Dim file_name As String = DTE.ActiveWindow.Document.FullName

        Dim GPUSAParserPath As String = "I:\arktos\WarOnline\src\Tools\Bin\GPUSAParser.exe"

        Dim result_file_path = System.IO.Path.GetTempPath() + "result.txt"

        Dim commandString = """" + GPUSAParserPath + """ """ + file_name + """ """ + result_file_path + """"

        If nvidia <> 0 Then
            If nvidia_short <> 0 Then
                commandString += " -short_nvidia"
            Else
                commandString += " -nvidia"
            End If
        End If

        If amd <> 0 Then
            commandString += " -amd"
        End If

        If sm = 1 Then
            commandString += " -sm20"
        End If

        If sm = 2 Then
            commandString += " -sm40"
        End If

        If sm = 3 Then
            commandString += " -sm50"
        End If

        For Each document As Document In DTE.Documents
            If document.FullName = result_file_path Then
                document.Close()
            End If
        Next

        If (System.IO.File.Exists(result_file_path)) Then
            System.IO.File.Delete(result_file_path)
        End If

        Dim full_command As String = "cmd /c """ + commandString + """"

        REM If nvidia = 0 Or nvidia_short <> 0 Then
        REM Shell(full_command, AppWinStyle.Hide, True, 10000)
        REM Else
        REM Shell(commandString, AppWinStyle.NormalFocus, False)
        REM End If

        Shell(full_command, AppWinStyle.Hide, False, 60000)

        While Not System.IO.File.Exists(result_file_path)
            System.Threading.Thread.Sleep(133)
        End While

CheckFileIsFree:

        Dim result_file As System.IO.FileStream

        Try
            result_file = System.IO.File.OpenWrite(result_file_path)

            result_file.Close()

        Catch E As Exception
            System.Threading.Thread.Sleep(133)
            GoTo CheckFileIsFree
        End Try

        DTE.ItemOperations.OpenFile(result_file_path)

    End Sub

    Sub AnalizeShaderAMD()
        DoAnalizeShader(0, 1, 0, 0)
    End Sub

    Sub AnalizeShaderAMD20()
        DoAnalizeShader(0, 1, 0, 1)
    End Sub

    Sub AnalizeShaderAMD40()
        DoAnalizeShader(0, 1, 0, 2)
    End Sub

    Sub AnalizeShaderAMD50()
        DoAnalizeShader(0, 1, 0, 3)
    End Sub


    REM Sub AnalizeShaderNVidia()
    REM     DoAnalizeShader(1, 0, 0)
    REM End Sub

    REM Sub AnalizeShaderAll()
    REM         DoAnalizeShader(1, 1, 0)
    REM End Sub

    REM Sub AnalizeShaderAllFast()
    REM         DoAnalizeShader(1, 1, 1)
    REM End Sub


End Module