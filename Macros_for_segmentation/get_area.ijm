dir = getDirectory("Choose a Directory ");
DIR = dir + "selected/"
OUT_cropped_image = dir + "steps/cropped/";
OUT_area = dir + "steps/Area/";

//FUNCTION
// This function opens the cropped image and select the polygon tool to pinpoint the area of the gill
function Area(input, output, filename) {
	
	open(input + filename);
	//run("Add Image...", "image=filename x=0 y=0 opacity=80");	
	waitForUser("Area selection, or Shift+click to skip");
	if (isKeyDown("Shift") == true) {
		run("Close All");
	}
	else {
	setTool("polygon");
	title="Pinpoint area";
	message="Paint the area of the gill with the polygon tool \n then click OK";
	waitForUser(title,message);
	saveAs("Selection", output + filename);
	//saveAs("Tiff", output2 + filename);
	
	run("Close All");
	}
}

// This applies the function to all the images in the cropped folder
ARRAY = getFileList(OUT_cropped_image);
for (i = 0; i < ARRAY.length; i++)
	Area(OUT_cropped_image, OUT_area, ARRAY[i]);
run("Close All");
