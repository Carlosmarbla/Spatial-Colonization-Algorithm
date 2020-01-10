dir = getDirectory("Choose a Directory ");
DIR = dir + "selected/"
OUT_skeleton = dir + "Skeleton/";
OUT_area = dir2 + "steps/Area/";
OUT_area2 = dir + "Skeleton_Area/";

//FUNCTION
// This function combines the skeleton file and the area .roi file and save it as a .pgm file
function Area(input, input2, output, filename) {
	//run("Image Sequence...", "open=input + filename number=1 sort")
	open(input + filename);
	name = File.nameWithoutExtension();
	string1= substring(name, 0,2);
	open(input2 + string1 + ".roi");
	setForegroundColor(255, 255, 255);
	run("Draw", "slice");
	saveAs("PGM", output + filename);
	run("Close All");
}

ARRAY = getFileList(OUT_skeleton);
ARRAY2 = getFileList(OUT_area);
for (i = 0; i < ARRAY.length; i++)
	Area(OUT_skeleton, OUT_area, OUT_area2, ARRAY[i]);
run("Close All");
