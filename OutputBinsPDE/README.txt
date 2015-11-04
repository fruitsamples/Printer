The OutputBinsPDE project demonstrates how to write a Cocoa PDE that supports PPD features and
that runs on Mac OS X versions 10.5 and later. It also contains code for handling the equivalent
support (minus PPD constraint handling) on 10.4. (Cocoa PDEs are not supported prior to 10.4)

# to build and install debug 4 way fat
sudo xcodebuild -configuration Debug DSTROOT=/ UNSTRIPPED_PRODUCT=YES install RC_ARCHS="ppc ppc64 i386 x86_64"

# to build and install release 4 way fat
sudo xcodebuild -configuration Release DSTROOT=/ install RC_ARCHS="ppc ppc64 i386 x86_64"

Either of these installs the OutputBinsPDE plugin and a PPD file that references it. The PPD is for a generic 
printer model called "OutputBins PDE test". This is a PPD for a PostScript printer but there is no reason it 
has to be. 

To test the OutputBinsPDE you have to create a print queue for it. Browse to a PostScript printer and assign 
this PPD manually. Just choose the printer and select "Other..." from the popup to choose a PPD by model. 
You can find the relevant PPD by typing "Output" into the text search field below the Other... menu. This will show
you the PPD for the "OutputBins PDE test" printer model. Click on that PPD and choose Add.
