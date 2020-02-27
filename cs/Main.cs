using System;
using System.Windows.Forms;
using Mal;

class Program
{
	[STAThread]
	public static void Main (string[] args)
	{
		Application.EnableVisualStyles();
		Application.Run(new Mal.Interpreter());
	}
}
