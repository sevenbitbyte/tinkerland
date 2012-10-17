package edu.rit.pj.parsec;

import java.util.List;
import java.util.ArrayList;
import java.util.Iterator;

public class Utils {
	static double dotProduct(List<Double> l1, List<Double> l2) {
		double out = 0.0;
		Iterator<Double> i1 = l1.iterator();
		Iterator<Double> i2 = l2.iterator();
		while (i1.hasNext() && i2.hasNext()) {
			out += i1.next() * i2.next();
		}
		return out;
	}
	static Double magnitude(List<Double> l) {
		return Math.sqrt(dotProduct(l, l));
	}
	static List<Double> addList(List<Double> l1, List<Double> l2) {
		List<Double> out = new ArrayList<Double>();
		Iterator<Double> i1 = l1.iterator();
		Iterator<Double> i2 = l2.iterator();
		while (i1.hasNext() && i2.hasNext()) {
			out.add(i1.next() + i2.next());
		}
		return out;
	}
	static List<Double> subtractList(List<Double> l1, List<Double> l2) {
		List<Double> out = new ArrayList<Double>();
		Iterator<Double> i1 = l1.iterator();
		Iterator<Double> i2 = l2.iterator();
		while (i1.hasNext() && i2.hasNext()) {
			out.add(i1.next() - i2.next());
		}
		return out;
	}
	static List<Double> multiplyList(List<Double> l, double scalar) {
		List<Double> out = new ArrayList<Double>();
		for (Iterator<Double> i = l.iterator(); i.hasNext(); ) {
			out.add(i.next() * scalar);
		}
		return out;
	}
	static List<Double> unitize(List<Double> l) {
		return multiplyList(l, 1/magnitude(l));
	}
}
