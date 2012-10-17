package edu.rit.pj.parsec;

import java.util.List;
import java.util.LinkedList;
import java.util.Iterator;
import java.lang.Double;

import edu.rit.pj.parsec.Utils;

public class Particle implements Cloneable {
	private String name;
	private int type;
	private List<Double> pos;
	private List<Double> momentum;
	private double mass;
	public Particle(String name, int type, List<Double> pos, List<Double> momentum, double mass) {
		this.name = name;
		this.type = type;
		this.pos = pos;
		this.momentum = momentum;
		this.mass = mass;
	}
	public String getName() {
		return name;
	}
	public int getType() {
		return type;
	}
	public List<Double> getPos() {
		return pos;
	}
	public List<Double> getMomentum() {
		return momentum;
	}
	public double getMass() {
		return mass;
	}
	public void stepParticle(List<Double> netforce, double dt) {
		List<Double> dmomentum = Utils.multiplyList(netforce, dt);
		
		// Increment momentum
		Iterator<Double> m_iter = momentum.iterator();
		Iterator<Double> d_iter = dmomentum.iterator();
		List<Double> buf = new LinkedList<Double>();
		while (m_iter.hasNext() && d_iter.hasNext()) {
			buf.add(m_iter.next() + d_iter.next());
		}
		momentum.clear();
		momentum.addAll(buf);
		
		// Increment position
		List<Double> dpos = Utils.multiplyList(momentum, dt/mass);
		m_iter = pos.iterator();
		d_iter = dpos.iterator();
		buf = new LinkedList<Double>();
		while (m_iter.hasNext() && d_iter.hasNext()) {
			buf.add(m_iter.next() + d_iter.next());
		}
		pos.clear();
		pos.addAll(buf);
	}
	public double getEnergy() {
		// 1/2 * m * v^2
		// 1/2 * (m*v)*(m*v)/m
		return 0.5 * Utils.dotProduct(momentum, momentum) / mass;
	}
}
