#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <math.h>




#define MAX_PACKAGES_PER_CLIENT  10
#define MAX_COORD                500
#define REGIONS                  9

// usage initial: rndmap <nombre de clients> <nombre de clusters> <capacit max des vehicule>
// usage actuel: rndmap <nombre de clients> <nombre de clusters>  // cree sa propore capacité d'un vehicule

typedef enum {FALSE=0 , TRUE=1}  bool;

typedef struct {
    unsigned int  x;
    unsigned int  y;
    int           cluster;
    int           to_deliver;
} Town;

typedef struct {
    unsigned int  xA;
    unsigned int  yA;
    unsigned int  xB;
    unsigned int  yB;
    int           to_deliver;
} Cluster;




Town    *towns;
Cluster *clusters;
int      created_towns;
int      created_clusters;
int      truck_capacity;
int      towns_to_create;
int      clusters_to_create;




unsigned int random_int(unsigned int max) {
    return (rand() % max);
}


// random entre a et b inclus
unsigned int rand_a_b(int a, int b){
    unsigned int c= b+1;
    return rand()%(c-a) +a;
}



void create_random_clusters(int how_many, unsigned int x, unsigned int y, unsigned int width, unsigned int height) {
    unsigned int  xA1, yA1, xB1, yB1,
                  xA2, yA2, xB2, yB2;
    int           how_many_to_1,
                  how_many_to_2;
    
    
    if (how_many >= 2) {
        if (width >= height) {
            // on fait une coupe verticale
            xA1= x;
            yA1= y;
            xB1= x + width/3 + random_int(width/3);
            yB1= y + height;
            xA2= xB1;
            yA2= y;
            xB2= x + width;
            yB2= y + height;
            how_many_to_1= ((double)(xB1-x) / width) * how_many + 0.5;
            how_many_to_2= how_many - how_many_to_1;
        } else {
            // on fait une coupe horizontale
            xA1= x;
            yA1= y;
            xB1= x + width;
            yB1= y + height/3 + random_int(height/3);
            xA2= x;
            yA2= yB1;
            xB2= x + width;
            yB2= y + height;
            how_many_to_1= ((double)(yB1-y) / height) * how_many + 0.5;
            how_many_to_2= how_many - how_many_to_1;
        }
        create_random_clusters(how_many_to_1, xA1, yA1, xB1-xA1, yB1-yA1);
        create_random_clusters(how_many_to_2, xA2, yA2, xB2-xA2, yB2-yA2);
    } else if (how_many == 1) {
        clusters[created_clusters].xA= x;
        clusters[created_clusters].yA= y;
        clusters[created_clusters].xB= x + width;
        clusters[created_clusters].yB= y + height;
        created_clusters++;
    }
}




void reinitialize_cluster_shipping(void) {
    int  i;
    
    for (i=0 ; i<clusters_to_create ; i++) {
        clusters[i].to_deliver= 0;
    }
}




bool create_random_towns(int how_many, unsigned int xmin, unsigned int xmax, unsigned int ymin, unsigned int ymax) {
    bool          result;
    int           c;
    unsigned int  x, y;
    int           packages;
    
    
    if (towns_to_create-created_towns < how_many) {
        how_many= towns_to_create - created_towns;
        result= FALSE;
    } else {
        result= TRUE;
    }
    
    while (how_many-- > 0) {
        x= xmin + random_int(xmax-xmin+1);
        y= ymin + random_int(ymax-ymin+1);
        towns[created_towns].x= x;
        towns[created_towns].y= y;
        
        if (created_towns == 0) {
            packages= 0;
            if (clusters_to_create <= 1)  c= 0;
                                    else  c= clusters_to_create;
        } else {
            packages= random_int(MAX_PACKAGES_PER_CLIENT) + 1;
            c= 0;
            while (x < clusters[c].xA || x > clusters[c].xB || y < clusters[c].yA || y > clusters[c].yB) c++;
        }
        
        towns[created_towns].cluster= c;
        towns[created_towns].to_deliver= packages;
        
        if (created_towns != 0) {
            clusters[c].to_deliver+= packages;
            if (clusters[c].to_deliver > truck_capacity  &&  clusters_to_create > 1)  result= FALSE;
        }
        created_towns++;
    }
    
    return (result);
}




int main(int argc, char *argv[]) {
    int   towns_to_randomly_put;
    bool  generate_homogeneous_map;
    int   towns_to_put;
    bool  retry;
    int   i;
    
    
    
    if (argc == 4) {
        generate_homogeneous_map= TRUE;
    }
    else if (argc == 5) {
         switch (argv[4][0]) {
             case 'h' : generate_homogeneous_map= TRUE;
                        break;
             case 'r' : generate_homogeneous_map= FALSE;
                        break;
             default  : return (EINVAL);
         }
    }
    else return (EINVAL);
    
    
    towns_to_create= atoi(argv[1]);
    clusters_to_create= atoi(argv[2]);
    truck_capacity= atoi(argv[3]);
    if (towns_to_create <= 0  ||  clusters_to_create < 0  ||  truck_capacity <= 0)  return (EINVAL);
    if (clusters_to_create == 0)  clusters_to_create= 1;
    
    srand((unsigned int)time(NULL));
    
    towns= calloc(towns_to_create, sizeof(towns[0]));
    clusters= calloc(clusters_to_create, sizeof(clusters[0]));
    
    if (towns == NULL  ||  clusters == NULL) {
        if (towns != NULL)  free(towns);
        if (clusters != NULL)  free(clusters);
        return (ENOMEM);
    }
    
    created_clusters= 0;
    create_random_clusters(clusters_to_create, 0, 0, MAX_COORD, MAX_COORD);
    
    do {
        if(generate_homogeneous_map == FALSE && towns_to_create >= 2*REGIONS) {
            int  average_towns_per_region;
            int  region_width;
            int  xmin, xmax,
                 ymin, ymax;
            
            
            average_towns_per_region= towns_to_create / REGIONS;
            region_width= MAX_COORD/sqrt(REGIONS);
            do {
                reinitialize_cluster_shipping();
                created_towns= 0;
                towns_to_put= towns_to_create;
                retry= FALSE;
                ymin= 0;
                while (ymin < MAX_COORD) {
                    ymax= ymin + region_width;
                    if (ymax > MAX_COORD) ymax= MAX_COORD;
                    xmin= 0;
                    while (xmin < MAX_COORD) {
                        xmax= xmin + region_width;
                        if (xmax > MAX_COORD)  xmax= MAX_COORD;
                        //                                   1,5*(nombre de villes dans une région complète * aire de la région à remplir / aire d'une région complète        )
                        towns_to_randomly_put= random_int(1 + 3 * average_towns_per_region * (unsigned long long)(xmax-xmin)*(ymax-ymin) / ((region_width)*(region_width)) / 2);
                        retry|= !create_random_towns(towns_to_randomly_put, xmin, xmax, ymin, ymax);
                        towns_to_put-= towns_to_randomly_put;
                        xmin= xmax;
                    }
                    ymin= ymax;
                }
                // si on a généré aléatoirement moins des 9/10 du nombre de villes demandées
                if (towns_to_put > towns_to_create/10)  retry= TRUE;
            } while (retry);
        } else {
            created_towns= 0;
            towns_to_put= towns_to_create;
        }
        // distribution aléatoire et homogène de  towns_to_put  villes (complète le travail du distributeur hétérogène)
        retry= !create_random_towns(towns_to_put, 0, MAX_COORD, 0, MAX_COORD);
	
    } while (retry);
    


	///////////////////////////////////////////////////////////////////////
	///////////// calcul de la capacité en adequate
	///////////////////////////////////////////////////////////////////////
	unsigned int max_demande, min_demande;
	max_demande=0;
	min_demande=0;
	
	// calcule de la borne max
	for (i=0 ; i<towns_to_create ; i++) {
		max_demande += towns[i].to_deliver;
	}
	
	// calcule de la borne min
	// intialisation
	unsigned int cluster_demande[created_clusters+1];
	for(i=0; i<= created_clusters ; i++){
		cluster_demande[i]= 0;	
	}
	// calcul de la demande de chaque cluster
	for(i=0 ; i<towns_to_create ; i++){
		cluster_demande[towns[i].cluster] += towns[i].to_deliver; 	
	}
	// recherche de la demande maximale
	for(i=0; i<= created_clusters ; i++){
		if(min_demande < cluster_demande[i]){
			min_demande=cluster_demande[i];	
		}
	}
	truck_capacity= rand_a_b(min_demande, max_demande);
    // sortie de la carte
	if (clusters_to_create <= 1)  created_clusters= 0;
		printf("NAME: instance-n%u-c%u-cap%u.ccvrp\n", towns_to_create, created_clusters+1, truck_capacity);
	    	//printf("# vertex %u  |  clusters %u  |  capacity %u  |  best path ?\n%u\t%u\t%u\n", towns_to_create, created_clusters+1, truck_capacity, towns_to_create-1, created_clusters, truck_capacity);
    	printf("TYPE: CCVRP\n");
	printf("COMMENT: auto-generated\n");
	printf("DIMENSION: %u\n", towns_to_create);
	printf("CAPACITY: %u\n", truck_capacity);
	printf("EDGE_WEIGHT_TYPE: EUC_2D_INT\n");
	printf("NODE_COORD_TYPE: TWOD_COORD\n");

	// cardianl positions
	printf("NODE_COORD_SECTION\n");

	for (i=0 ; i<towns_to_create ; i++) {
		printf("%u %u %u\n", i+1, towns[i].x, towns[i].y);
	}

	// damande for each customer
	printf("DEMAND_SECTION\n");
	for (i=0 ; i<towns_to_create ; i++) {
		printf("%u %u\n", i+1, towns[i].to_deliver);
	}
	
	// cluster of each customer
	printf("CLUSTER_SECTION\n");
	for (i=0 ; i<towns_to_create ; i++) {
		printf("%u %u\n", i+1, towns[i].cluster);
	}

	// the depot
	printf("DEPOT_SECTION\n1\n-1\nEOF\n");
	/*
	for (i=0 ; i<towns_to_create ; i++) {
	printf("%u\t%u\t%u\t%u\n", towns[i].x, towns[i].y, towns[i].cluster, towns[i].to_deliver);

	}
	*/
	
	free(towns);
	free(clusters);

	return (0);
}



