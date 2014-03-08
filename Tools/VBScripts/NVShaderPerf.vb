'
' This code should go into a VS Macro.
' IMPORTANT : make sure macro module name matches module name in this code (NVShaderPerf currently)
' IMPORTANT : change 'C:\Program Files\NVIDIA Corporation\NVIDIA ShaderPerf\' below to your NVShaderPerf path
'

Imports System
Imports EnvDTE
Imports EnvDTE80
Imports EnvDTE90
Imports System.Security.AccessControl
Imports System.Threading.Thread
Imports System.Diagnostics

Public Module NVShaderPerf

    Sub NVShaderPerfCurFile()

        Dim file_name As String = DTE.ActiveWindow.Document.FullName

        Dim command_args As String

        If file_name.Contains("_vs") Then
            command_args = " -type cg_vp "
        Else
            command_args = " -type cg_fp "
        End If

        command_args = command_args + "-g rsx "

        Dim NVPerfPath As String = "C:\Program Files\NVIDIA Corporation\NVIDIA ShaderPerf\"

        Dim commandString = """" + NVPerfPath + "NVShaderPerf.exe""" + command_args + """" + file_name + """"

        Dim result_file_path = NVPerfPath + "nvperf_out.txt"

        For Each document As Document In DTE.Documents
            If document.FullName = result_file_path Then
                document.Close()
            End If
        Next

        If (System.IO.File.Exists(result_file_path)) Then
            System.IO.File.Delete(result_file_path)
        End If

        Dim full_command As String = "cmd /c """ + commandString + " > " + """" + result_file_path + """"""
        Shell(full_command, AppWinStyle.Hide, True, 5000)

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



End Module
