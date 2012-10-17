package edu.rit.pj.parsec;

import java.util.List;
import java.util.ArrayList;
import java.util.Iterator;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.FileReader;
import java.io.FileNotFoundException;
import java.io.IOException;

import edu.rit.pj.parsec.Particle;

public class Parsec {
	private static Particle[] particles = new Particle[0];
	private static int[][] graph = new int[0][0];
	
	private Parsec() {}
	
	private static void readInData(String relationshipfilename) throws IOException, FileNotFoundException {
		String relbuf, posbuf;
		List<Particle> particles_l = new ArrayList<Particle>();
		List<int[]> graph_l = new ArrayList<int[]>();
		
		BufferedReader relationsreader = new BufferedReader(new FileReader(relationshipfilename));
		BufferedReader positionsreader = new BufferedReader(new InputStreamReader(System.in));
		while ((relbuf = relationsreader.readLine()) != null &&
				(posbuf = positionsreader.readLine()) != null) {
			if (relbuf.isEmpty() || posbuf.isEmpty()) {
				break;
			}
			String[] relsplit = relbuf.split(",");
			String[] possplit = posbuf.split(",");
			List<Double> position = new ArrayList<Double>();
			List<Double> momentum = new ArrayList<Double>();
			for (String s : possplit) {
				position.add(Double.parseDouble(s));
				momentum.add(0.0);
			}
			particles_l.add(new Particle(relsplit[2],
					Integer.parseInt(relsplit[0]),
					position, momentum, 1.0));
			int[] adjacencies = new int[relsplit.length-3];
			for (int i = 3; i < relsplit.length; i++) {
				adjacencies[i-3] = Integer.parseInt(relsplit[i]);
			}
			graph_l.add(adjacencies);
		}
		particles = particles_l.toArray(particles);
		graph = graph_l.toArray(graph);
	}
	
	private static void runSimulation(final double dt) {
		final double threshold = 0.01;
		final double spring_k = 1.0;
		final double gravity_g = 0.005;
		final double friction_k = 0.05;
		int iterations = 0;
		double maxenergy;
		Particle[] newparticles;
		do {
			newparticles = new Particle[particles.length];
			// Calculate the net force on particle p
			for (int i = 0; i < particles.length; i++) {
				List<Double> netforce = new ArrayList<Double>();
				netforce.add(0.0);
				netforce.add(0.0);
				netforce.add(0.0);
				// Calculate attracting force
				// f = -kx for particles attached in the graph
				// This can be parallelized a bunch.
				for (int j = 0; j < particles.length; j++) {
					if (j == i) {
						for (int k = 0; k < graph[j].length; k++) {
							List<Double> diff = Utils.subtractList(particles[graph[j][k]].getPos(),
									particles[i].getPos());
							netforce = Utils.addList(netforce,
									Utils.multiplyList(Utils.unitize(diff),
									spring_k * Utils.magnitude(diff)));
						}
					}
					else {
						for (int k = 0; k < graph[j].length; k++) {
							if (graph[j][k] == i) {
								List<Double> diff = Utils.subtractList(particles[j].getPos(),
										particles[i].getPos());
								netforce = Utils.addList(netforce,
										Utils.multiplyList(Utils.unitize(diff),
										spring_k * Utils.magnitude(diff)));
							}
						}
					}
				}
				// Calculate repelling force
				// f = g*m1*m2/r^2
				for (int j = 0; j < particles.length; j++) {
					if (j != i) {
						List<Double> diff = Utils.subtractList(particles[i].getPos(), particles[j].getPos());
						netforce = Utils.addList(netforce, 
								Utils.multiplyList(Utils.unitize(diff), 
										gravity_g*particles[i].getMass()*particles[j].getMass()/
										Math.pow(Utils.magnitude(diff), 2)));
					}
				}
				// Calculate friction
				// f = mew_k * n = mew_k * m * g = friction_k * m
				if (Utils.magnitude(particles[i].getMomentum()) > 0.0) {
					List<Double> diff = Utils.multiplyList(Utils.unitize(particles[i].getMomentum()), -1.0);
					netforce = Utils.addList(netforce, 
							Utils.multiplyList(diff, friction_k * particles[i].getMass()));
				}
				// Step the particle
				newparticles[i] = new Particle(particles[i].getName(),
						particles[i].getType(),
						particles[i].getPos(),
						particles[i].getMomentum(),
						particles[i].getMass());
				newparticles[i].stepParticle(netforce, dt);
				Iterator<Double> iter = newparticles[i].getPos().iterator();
				for (int j = 0; j < newparticles[i].getPos().size(); j++) {
					System.out.print(iter.next());
					if (j < newparticles[i].getPos().size() - 1) {
						System.out.print(",");
					}
				}
				System.out.println();
			}
			particles = newparticles;
			// Calculate termination condition
			maxenergy = 0.0;
			for (Particle p : particles) {
				double energy = p.getEnergy();
				maxenergy = energy > maxenergy ? energy : maxenergy;
			}
			iterations++;
		} while (/*iterations < 50*/maxenergy > threshold);
		System.out.println();
	}
	
	public static void main(String[] args) {
		if (args.length != 1) {
			System.err.println("Usage: java Parsec relationshipsfile < positionsfile");
			System.exit(-1);
		}
		try {
			readInData(args[0]);
		}
		catch (FileNotFoundException e) {
			e.printStackTrace();
			System.exit(-1);
		}
		catch (IOException e) {
			e.printStackTrace();
			System.exit(-1);
		}

		/*for (int i = 0; i < particles.length; i++) {
			Particle p = particles[i];
			System.out.print("Particle "+p.getName()+" has type "+p.getType()+" and is adjacent to [");
			for (int j : graph[i]) {
				System.out.print("Particle "+j+" ");
			}
			System.out.println("]");
		}*/
		runSimulation(0.1);
	}

}
