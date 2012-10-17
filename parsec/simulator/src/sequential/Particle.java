//package sequential;

import java.util.List;
import java.util.LinkedList;
import java.util.Iterator;
import java.lang.Double;

//import sequential.Utils;

public class Particle implements Cloneable {
	public String name;
	public int type;
	public double pos[]={0.0, 0.0, 0.0};
	public double momentum[]={0.0, 0.0, 0.0};
	public double momentumSqrSum=0.0;
	public double mass=1.0;
	public double energy=0.0;

	public Particle(String name, int type){
		this.name=name;
		this.type=type;
	}

	public Particle(Particle other){
		name=other.name;
		type=other.type;
		mass=other.mass;
		energy=other.energy;
		this.copy(other);
	}

	public Particle(String name, int type, double pos[], double momentum[], double mass) {
		this.name = name;
		this.type = type;
		this.pos = pos;
		this.momentum = momentum;
		this.mass = mass;
	}

	public void copy(Particle other){
		momentumSqrSum=other.momentumSqrSum;
		for(int i=0; i<3; i++){
			pos[i]=other.pos[i];
			momentum[i]=other.momentum[i];
		}
	}

	public void stepParticle(double netforce[], double dt) {
		double dtMass=dt/mass;
		momentumSqrSum=0.0;
		for(int i=0; i<3; i++){
			momentum[i]+=netforce[i]*dt;
			pos[i]+=momentum[i]*dtMass;
			momentumSqrSum+=momentum[i]*momentum[i];
		}
		energy=0.5*momentumSqrSum/mass;
	}

	/*public double getEnergy() {
		// 1/2 * m * v^2
		// 1/2 * (m*v)*(m*v)/m
		//return 0.5 * Utils.dotProduct(momentum, momentum) / mass;
		return energy;
	}*/
}
