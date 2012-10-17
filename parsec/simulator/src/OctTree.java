package edu.rit.pj.parsec;

import java.util.*;
import edu.rit.pj.Comm;
import edu.rit.pj.*;
import edu.rit.pj.parsec.Particle;

/* 
 * OctTree - 3D quad tree, used here to optimize the calculation of 
 * repulsive forces in n-body type simulations. 
 *
 * At this time restructuring is done by rebuilding the tree
 */

public class OctTree {

    /* ORDER OF OCTANTS, counter clockwise starting at y=0, x axis
       0: NEE
       1: NNE
       2: NNW
       3: NWW
       4: SWW
       5: SSW
       6: SSE
       7: SEE
    */
    private OctTree children[]; 

    /* List of Particles per quadrant. Correspond to octant order */
    private List<List<Particle>> data[];

    private static final int MAX_NODES = 5;   //maximum nodes per octant
    private static final MAX_DEPTH = 50;  //the deepest we will go
    private int level;       //how deep are we?  0 for root

    /* Minimum boundaries for this particle (-maximum if root) */
    private int min_x; 
    private int min_y;
    private int min_z;

    /* Maximum boundaries for this particle (maximum if root) */
    private int max_x; 
    private int max_y;
    private int max_z;

    

    /*
     * Beyond the root
     */
    private OctTree( int l, Particle p ) {
        level = l;
        children = new OctTree[8];
        data = new ArrayList<<ArrayList<Particle>>();
        max_x = Integer.MAX_VALUE/l;
        max_y = Integer.MAX_VALUE/l;
        max_z = Integer.MAX_VALUE/l;
        min_x = -max_x;
		min_y = -max_y;
		min_z = -max_z;
        
    }

    /*
     * init
     */
    public OctTree() {
        children = new OctTree[8];
        data = new ArrayList<<ArrayList<Particle>>();
        level = 0;
        max_x = Integer.MAX_VALUE;
        max_y = Integer.MAX_VALUE;
        max_z = Integer.MAX_VALUE;
        min_x = -max_x;
		min_y = -max_y;
		min_z = -max_z;
    }

    /*
     * Looks at each particle's position and adds to the appropriate 
     * quadrant. If full, sub-divide again and pass this particle.
     */
    public void addData(Particle p) { 
        //while level < 50
        //for this particle, attempt to add to list at appropriate octant
        //if not initialized, obv fill in a list
        //if octant !full, add particle  
        //else  - repartition
           //if child does not exist, create and pass particle
           //else recurse with subtree
       while( level < 50 ) {
           int oct = findOctant(p);
           if( ArrayList<Particle> tmp1 = data.get(oct) == null ) {
                   tmp1 = new ArrayList<Particle>();
           }
           List<Particle> lst = data.get(oct);
           int size = lst.size();
           if (size < MAX_NODES ) { 
               lst.add(oct, p);
               data.set(oct, lst); //replace list
           } else { 
               children[oct] = new OctTree( level + 1, p );
               children[oct].add(oct, p);
               //think this works..
               children[oct].data.set(oct, lst); 
           }
       }
    }

    // which to choose based on position?
    public int findOctant( Particle p ) {
        return 0;
    }
    
    // Recurse through whole tree from this level and position
    // return all particles in tree from here
    // could pass in strings: NNE, SWW, etc.. and just check/convert to int
    public List<Particle> getValues( int level, int pos ) { 
        //arghghgg

    }

}

