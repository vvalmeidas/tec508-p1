#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "csv.h"

static const int NUM_IMAGES_TESTING = 10074;
static const int NUM_IMAGES_TRAINING = 8092;
static const int NUM_PIXELS = 2304;


void *emalloc (size_t n);
char *estrdup (char *s);


int main (int argc, char *argv[]) {

    /* import a delimited text file as a new dataset */

    FILE *ifp;
    char **varnames;
    int nvars = 0;
    char *line;
    float **data_testing;
    float **data_training;
    float *labels;

    /* try to open file */
    if ((ifp = fopen("../data/fer2013.csv", "r")) == NULL) {
        printf("Could not open the file!");
        return -1;
    } else {
        printf("File is open!\n");
    }

    /***
        read variable names from first row
    ***/
    if (csvgetline(ifp, ',', 0) == NULL) {
        printf("File is empty!");
        return -1;
    }

    /* set number of variables to number of fields parsed */
    if ((nvars = csvnfield()) < 1) {
        printf("Error:  No variable names found.  Check that delimiter string is correct.");
        return -1;
    }

    printf("%s%d \n", "Number of variables found: ", nvars);

    /* allocate space to store variable names */
    varnames = (char **) emalloc(nvars * sizeof(char *));

    /* read in varnames */
    printf("Variable names: ");
    for (int i = 0; i < nvars; i++) {
        varnames[i] = estrdup(csvfield(i));
        printf("%s ", varnames[i]);
    }

    data_testing = (float **) emalloc(NUM_IMAGES_TESTING * sizeof(float));
    data_training = (float **) emalloc(NUM_IMAGES_TRAINING * sizeof(float));

    int row_testing = 0; //data rows count
    int row_training = 0; //training data rows count
    char *label, *pixels, *usage, *pch; 
    float temp; //temp value to be stored on the data

    int cont = 0;

    

    /* iterate until the end of the file, reading every line */
    while(((line = csvgetline(ifp, ',', 0)) != NULL)) {
        label = estrdup(csvfield(0)); //gets the label


        if((strcmp(label, "0") == 0 || strcmp(label, "2") == 0)) { 
            pixels = estrdup(csvfield(1)); //gets the pixel
            usage = estrdup(csvfield(2)); //gets the usage
            
            pch = strtok (pixels," ");

            if(strcmp(usage, "Training") == 0) {
                data_training[row_training] = (float *) emalloc(NUM_PIXELS * sizeof(float));
            } else {
                cont++;
                data_testing[row_testing] = (float *) emalloc(NUM_PIXELS * sizeof(float));
            }

            for(int c = 0; c < NUM_PIXELS; c++) {
                temp = strtof(pch, NULL)/255;
                data_testing[row_testing][c] = temp;
                                
                if(strcmp(usage, "Training") == 0) {
                    data_training[row_training][c] = temp;
                }

                pch = strtok (NULL, " ");
            }

            if(strcmp(usage, "Training") == 0) {
                row_training++;
            }

            row_testing++;
        }
    }

    printf("Testing: %d\n", cont);

    //printf("%f %f %f %f\n", data[0][0], data[0][1], data[0][2], data[0][3]);
}

void *emalloc (size_t n) {

	void *p;

	p = malloc(n);
	if (p == NULL) {
		printf("Fatal error!  malloc of %u bytes failed:", (unsigned int) n);
	}
	return p;

}

char *estrdup (char *s) {

	char *t;

	t = (char *) malloc(strlen(s)+1);
	if (t == NULL) {
		printf("Fatal error!  estrdup(\"%.20s\") failed:", s);
	}
	strcpy(t, s);
	return t;

}
