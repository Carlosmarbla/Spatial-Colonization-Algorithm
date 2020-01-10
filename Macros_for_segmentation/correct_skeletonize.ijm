dir = getDirectory("Choose a Directory ");
DIR = dir + "selected/"
OUT_cropped_image = dir + "steps/cropped/";
OUT_classified_image = dir + "steps/classified/";
OUT_BnW_class = dir + "steps/BnW/";
OUT_BnW_corrected = dir + "steps/BnW_corrected/";
OUT_skeleton = dir + "Skeleton/";
OUT_oriented_image = dir + "steps/Oriented/";
steps = dir + "steps/";

//FUNCTIONS 
// This function opens the classified image, converts it to 8-bit, opens the cropped and classified image together to manually correct the tracheal tree
function Black_FLAG(input, input2, output, output2, filename) {
	open(input2 + filename);
	open(input + filename);
	setThreshold(0, 0);
	setForegroundColor(255, 255, 255);
	run("Convert to Mask");
	run("RGB Color");
	run("Invert");
	run("8-bit");
//	open(input + filename); // open cropped image to compare and then close it
	//run("Threshold...");
	id=getImageID(); 
	waitForUser("Manual correction, or Shift+click <<ok>> to skip");
	if (isKeyDown("Shift") == true) {
		run("Close All");
	}
	else {
	setTool( "Pencil Tool" ); // change this to pencil an set black or white colour before
	 // open cropped image to compare and overlay with some transparency, and then close it once manual correction is finished before saving the corrected image
	run("Add Image...", "image=filename x=0 y=0 opacity=60");	
	title="Connect branchess";
	message="Paint posible path for disconnected TRUE trachea \n then click OK";
	waitForUser(title,message);
	//run("Paintbrush Tool Options", "brush=1");
	//setTool(16);
	setForegroundColor(0, 0, 0);
	setTool( "Pencil Tool" );
	title="Disconnect False Positives";
	message="Remove connection of false trachea \n //NOTE: Pixels may contact by their corners \n Then click OK"; //NOTE: Pixels may contact by their corners if allowed diagonal connections
	waitForUser(title,message);
	
// close cropped_example_image before saving the segmented corrected image
	close(input2 + filename);
// SAVE A COPY OF THE MANUALLY corrected PART
	saveAs("Tiff", output2 + filename);

// This fill any "holes" in the tracheas, and erase anything that is not connected to the tracheas	
	run("Find Connected Regions", "allow_diagonal display_one_image regions_for_values_over=150 minimum_number_of_points=150 stop_after=1");

	//t=getTitle();s=lastIndexOf(t, '_');t=substring(t, 0,s);//t=replace(t," ","_"); // change title (from last point remove all)
	//t2= t +'BnW'; // CHANGE HERE THE TITLE
	setAutoThreshold("Default dark");
	saveAs("Tiff", output + filename);
	run("Close All");
	}
	
}


// This function open the classified image, ir does a skeletonization and change it to the desired colour for future analysis
function Fill_and_skeletonize(input, output, filename) {
	open(input + filename);
//setTool("wand");
	//setAutoThreshold("Default dark");
//run("Wand Tool...");
	run("Convert to Mask");
	run("Fill Holes");
	run("Skeletonize");
	run("Invert");
	run("RGB Color");
	saveAs("PGM", output + filename);
	run("Close All");
}


ARRAY = getFileList(OUT_classified_image);
for (i = 0; i < ARRAY.length; i++)
	Black_FLAG(OUT_classified_image, OUT_cropped_image, OUT_BnW_class, OUT_BnW_corrected, ARRAY[i]);

ARRAY = getFileList(OUT_BnW_class);
for (i = 0; i < ARRAY.length; i++)
	Fill_and_skeletonize(OUT_BnW_class, OUT_skeleton, ARRAY[i]);
run("Close All");
