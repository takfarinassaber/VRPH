#include <iostream>
#include <vector>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cmath>

using namespace std;
#define BIGM 10000

typedef struct _node {
  double x;
  double y;
  int cluster;
  string id;
} node;
/**
 * transforme a VRP instance with distances computed according to the euclidiean 
 * position of each noeud
 * to a VRP instance with explicit distances (in a FULL_MATRIX format)
 * @param euc_2D_file: file with distances in euclidiean format
 */
FILE * matrix_creation(char* euc_2D_file, char * output)
{
  bool is_explicit;
  vector< vector<double> > matrix;
  vector<node> nodes;
  
	
  FILE* infile= fopen(euc_2D_file,"r");
  if(infile == NULL)
    {
      fprintf(stderr,"Unable to open %s for reading\n", euc_2D_file);
      exit(-1);

    }
	
  // the file that will contain the same instance with distances between noeuds in matrix
  ofstream outfile(output);
	
	
  // 	
  int dimension=0;
	
  //char str1[VRPH_STRING_SIZE];
  char str1[1000];
  fscanf(infile,"%s",str1);
  
  while(strncmp(str1,"EOF",3)!=0)
    {
      //cout << str1 << endl;
      if(strncmp(str1, "DIMENSION:",10)==0)
        {
	  // the savegarde of the dimension of the problem
	  fscanf(infile,"%d\n",&dimension);
	  outfile << "DIMENSION: " << dimension << endl;
	} 
      else if(strncmp(str1, "DIMENSION",9)==0)
	{
	  fscanf(infile,"%s",str1); // extract the ":"
	  fscanf(infile,"%d\n",&dimension);
	  outfile << "DIMENSION: " << dimension << endl;
	}
		
      // the modification of the type and the format of the instance
      else if(strncmp(str1, "EDGE_WEIGHT_TYPE:",17)==0){
	fscanf(infile, "%s", str1);
	outfile << "EDGE_WEIGHT_TYPE: "<< "EXPLICIT" << endl;
      }
      else if(strncmp(str1, "EDGE_WEIGHT_TYPE",16)==0){
	fscanf(infile,"%s",str1); // extract the ":"
	//char ** test;
	//getline(infile,test);
	fscanf(infile, "%s", str1);
	is_explicit = !strcmp(str1, "EXPLICIT");
			
			
	outfile << "EDGE_WEIGHT_TYPE: "<< "EXPLICIT" << endl;
      }
		
      else if(strncmp(str1, "EDGE_WEIGHT_FORMAT:",19)==0){
	fscanf(infile, "%s", str1);
	outfile << "EDGE_WEIGHT_FORMAT: "<< "FULL_MATRIX" << endl;
      }
      else if(strncmp(str1, "EDGE_WEIGHT_FORMAT",18)==0){
	fscanf(infile,"%s",str1); // extract the ":"
	fscanf(infile, "%s", str1);
	outfile << "EDGE_WEIGHT_FORMAT: "<< "FULL_MATRIX" << endl;
      }
      else if(strncmp(str1, "EDGE_WEIGHT_SECTION",19)==0){
	int i,j;
	
	for(i = 0; i<dimension; i++){
	  vector<double> v;
	  matrix.push_back(v);
	  for(j=0; j<dimension; j++){
	    double tmp;
	    fscanf(infile, "%lf", &tmp);
	    matrix.at(i).push_back(tmp);
	  }
	}

      }
      else if(strncmp(str1, "NODE_COORD_SECTION",19)==0){
	int i;
	outfile << "NODE_COORD_SECTION"<< endl;
	for(i=0; i<dimension; i++){
	  node tmp ;
	  char str_tmp[1024];
	  fscanf(infile, "%s %lf %lf", str_tmp, &tmp.x, &tmp.y);
	  tmp.id += string(str_tmp);
	  nodes.push_back(tmp);
	  outfile << "" << tmp.id << " " << tmp.x << " " << tmp.y << endl; 
	}
      }
      else if(strncmp(str1, "CLUSTER_SECTION",15)==0){
	int i;
	vector<node>::iterator it;
	for(i=0; i<dimension; i++){
	  char tmp[1024];
	  char tmp_id[1024];
	  char tmp_cluster[1024];
	  fscanf(infile, "%s %s", tmp_id, &tmp_cluster);
	 
	  //printf("%s %s", tmp_id, tmp_cluster);
	  bool find = false;
	  
	  for(it = nodes.begin(); it < nodes.end(); it++){
	    string tmp_id_in_string = string(tmp_id);
	    if (tmp_id_in_string.compare((*it).id) == 0){
	      (*it).cluster = atoi(tmp_cluster);
	      break;
	    }
	  }	 
	}

      }
      else {

	outfile << str1;
	fgets(str1, 1024, infile);
	outfile << str1;
      }
	
      fscanf(infile,"%s",str1);		

    }
  outfile << "EDGE_WEIGHT_SECTION" << endl;
  int i, j;
  if(is_explicit){
    
    for(i=0; i<dimension; i++){
      for(j=0; j<dimension; j++){
	if(nodes.at(i).cluster == nodes.at(j).cluster){
	  outfile << "" << matrix.at(i).at(j) << " ";
	}
	else {
	  // penality
	  outfile << "" << BIGM * matrix.at(i).at(j) << " ";
	}
      }
      outfile << endl;
    }
  }
  else{
    for(i=0; i<dimension; i++){
      for(j=0; j<dimension; j++){
	double dist = 
	  sqrt(pow(nodes.at(i).x - nodes.at(j).x, 2) + pow(nodes.at(i).y - nodes.at(j).y, 2));
	 
	if(nodes.at(i).cluster == nodes.at(j).cluster){
	  outfile << "" << dist << " ";
	}
	else {
	  // penality
	  outfile << "" << BIGM * dist << " " ;
	}
      }
      outfile << endl;
    }
  }
  outfile << "EOF";
  fclose(infile);
}

int main(int argc, char** argv)
{
  if(argc != 3){
    printf("Usage :\n");
    printf("Ce programme transforme une instance de type clustered vrp en une instance  vrp dont les distances entre noeuds des differents clusters ont ete penalisees\n");
    printf("%s <input> <output>\n", argv[0]);
    printf("%s explicit_full_avec_cluster.vrp sortie.vrp\n", argv[0]);
    exit(0);
  }
  matrix_creation(argv[1], argv[2]);
  return 0;
}

