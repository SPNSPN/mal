using System;
using System.Windows.Forms;
using Senva;

class Program
{
	[STAThread]
	public static void Main (string[] args)
	{
		Application.EnableVisualStyles();
		Application.Run(new Senva.Interpreter());
	}
}
