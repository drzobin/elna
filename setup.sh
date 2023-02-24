ELNA_HOME="/home/drz/github/elna/"
SEEDFILES_FOLDER=$ELNA_HOME"seedfiles/"
RESULTS_FOLDER=$ELNA_HOME"results/"
WORKING_DIR=$ELNA_HOME"working_dir/"

mkdir -p $SEEDFILES_FOLDER
mkdir -p $RESULTS_FOLDER
mkdir -p $WORKING_DIR

cp $ELNA_HOME"test.txt" $SEEDFILES_FOLDER

echo "For testing you can run: "
echo $ELNA_HOME"elna -s "$SEEDFILES_FOLDER" -o "$RESULTS_FOLDER" -w "$WORKING_DIR"test.txt "$ELNA_HOME"crash_me @@"