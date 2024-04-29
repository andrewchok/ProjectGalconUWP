# ProjectGalconUWP
Port of ProjectGalcon to UWP

To run this project you will need to provide a certificate.

While in Visual Studios:

In the Solution Explorer go to 
	Game > Game.Universal (Universal Windows) > Util > Package.appxmanifest
and open it

at the top switch to "Packaging"
	Application		Visual Assets	Capabilities	Declarations	Content URIs	[Packaging]
	
then select "Choose Certificate..." and follow the steps
	{usually choose "Create..." if you have no existing one}
