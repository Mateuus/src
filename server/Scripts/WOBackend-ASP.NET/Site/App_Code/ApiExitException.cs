using System;
using System.Collections.Generic;
using System.Web;

/// <summary>
/// Summary description for ApiExitException
/// </summary>
public class ApiExitException : SystemException
{
	public ApiExitException(string msg) : base(msg)
	{
	}
}
