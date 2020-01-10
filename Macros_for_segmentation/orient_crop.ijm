// INPUT AND OUTPUT DIRECTORIES
dir = getDirectory("Choose a Directory ");
DIR = dir + "selected/"
OUT_cropped_image = dir + "steps/cropped/";
OUT_classified_image = dir + "steps/classified/";
OUT_BnW_class = dir + "steps/BnW/";
OUT_BnW_corrected = dir + "steps/BnW_corrected/";
OUT_skeleton = dir + "Skeleton/";
OUT_oriented_image = dir + "steps/Oriented/";
OUT_area = dir + "steps/Area/";
OUT_area2 = dir + "Skeleton_Area/";
steps = dir + "steps/";
// CREATE DIRECTORIES
File.makeDirectory(steps);
File.makeDirectory(OUT_cropped_image);
File.makeDirectory(OUT_classified_image);
File.makeDirectory(OUT_BnW_class);
File.makeDirectory(OUT_BnW_corrected);
File.makeDirectory(OUT_skeleton);
File.makeDirectory(OUT_oriented_image);
File.makeDirectory(OUT_area);
File.makeDirectory(OUT_area2);
// DEFINED DISTANCES TO SET SCALES
tenXdistance = 0.9767 // this is for 10x images taken in Acaimo's lab microscope
twentyXdistance=  1.9535 // this is for 20x images taken in Acaimo's lab microscope

// FUNCTIONS
// This function sets the scale to the images and calls rotation of the image if necessary
function orient(input, output, filename) {
	open(input + filename);
	//run("Set Scale...", "distance=tenXdistance known=1 pixel=1 unit=micron");
	setForegroundColor(157, 157, 157);
	run("Canvas Size...", "width=1692 height=1692 position=Center");
	//title="Rotate Image";
	//message="rotate the image";
	run("Rotate... ");
	floodFill(0, 0);floodFill(1690, 1690);floodFill(0, 1690);floodFill(1690, 0);
	//waitForUser(title,message);
	saveAs("Tiff", output + filename);
	run("Close All");
}
// This function crops the image to the defined size
function crop(input, output, filename) {
	open(input + filename);
	waitForUser("Crop, or Shift+click <<ok>> to skip");
	if (isKeyDown("Shift") == true) {
		run("Close All");
	}
	else {
	title="UpLeftCorner";
	message="Select upperleft corner \n of the square to crop the image";
	setTool("point");
	waitForUser(title,message);
	s = selectionType();
	if( s == -1 ) {
	    exit("There was no selection.");
	} else if( s != 10 ) {
	    exit("The selection wasn't a point selection.");
	} else {
    	getSelectionCoordinates(xPoints,yPoints);
    	x = xPoints[0];
    	y = yPoints[0];
    	//showMessage("Got coordinates ("+x+","+y+")");
	}
	makeRectangle(x, y, 846, 846); // this changes depending on the objective used to take the image. In this case is for 10x objectives
	run("Crop");
	//run("Properties...");
	run("Scale...", "x=- y=- width=512 height=512 interpolation=Bilinear average create");
	saveAs("Tiff", output + filename);
	run("Close All");
}
}

// APLICATION OF FUNCTIONS
// This calls orient function in the desired folder
waitForUser("Orient images, or Shift+click <<ok>> to skip");
	if (isKeyDown("Shift") == true) {
		run("Close All");
	}
	else {
	ARRAY = getFileList(DIR);
	for (i = 0; i < ARRAY.length; i++)
		orient(DIR, OUT_oriented_image, ARRAY[i]);
} 

// This calls crop function in the desired folder
ARRAY = getFileList(OUT_oriented_image);
for (i = 0; i < ARRAY.length; i++)
	crop(OUT_oriented_image, OUT_cropped_image, ARRAY[i]);

run("Close All");
	

