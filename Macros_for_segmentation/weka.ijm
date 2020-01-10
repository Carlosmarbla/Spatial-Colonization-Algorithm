//DIRECTORIES
dir = getDirectory("Choose a Directory ");
MODEL = File.openDialog("Choose a Model"); 
OUT_cropped_image = dir + "steps/cropped/";
OUT_classified_image = dir + "steps/classified/";


// FUNCTION
// This function calls Trainable Weka Segmentation using a previous trained classifier model
function WEKA(input, output, filename) {
	open(input + filename);
	//setTool("freeline");
	run("Trainable Weka Segmentation");
	wait(3000);
	//selectWindow("Trainable Weka Segmentation v3.2.17");
	call("trainableSegmentation.Weka_Segmentation.loadClassifier", MODEL); // LOAD CLASSIFIER HERE
	call("trainableSegmentation.Weka_Segmentation.getResult");
	//selectWindow("Classified image");
	saveAs("Tiff", output + filename);
	run("Close All");
}

// APLICATION OF FUNCTION
// Applies the function to all the images in the desired folder
ARRAY = getFileList(OUT_cropped_image);
for (i = 0; i < ARRAY.length; i++)
	WEKA(OUT_cropped_image, OUT_classified_image, ARRAY[i]);
