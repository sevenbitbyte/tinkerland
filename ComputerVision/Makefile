CXXFLAGS = -ggdb
CFLAGS = -ggdb
LDFLAGS=-ggdb
CXXLIBS= -lm -lopencv_highgui -lopencv_core -lopencv_imgproc

#HELPER_FILES = lkt.cpp timer.cpp
#HELPER_OBJS = ${HELPER_FILES:%.cpp=%.o}

all: motion glcm keypoints laserMouse checkerboard goodfeatures crosshairs normThresh shapeTest colorThresh lucaskanade gjsolver motionseg floodFillSegment floodFillSegment1Chan colorThresh1Chan blurEdges houghLines perspective

perspective: perspective.o
	g++ -o perspective -ggdb perspective.o -L /usr/lib/ -lm ${CXXLIBS}

houghLines: houghLines.o
	g++ -o houghLines -ggdb houghLines.o -L /usr/lib/ -lm ${CXXLIBS}

blurEdges: blurEdges.o
	g++ -o blurEdges -ggdb blurEdges.o -L /usr/lib/ -lm ${CXXLIBS}

motionseg: MotionSeg/motionseg.o MotionSeg/affineModel.o MotionSeg/kdnode.h
	g++ -o MotionSeg/motionseg  MotionSeg/motionseg.o MotionSeg/affineModel.o -I MotionSeg/ -lgsl -lgslcblas ${CXXLIBS} -ggdb

gjsolver: MotionSeg/gjsolver.o MotionSeg/affineModel.o
	g++ -o MotionSeg/gjsolver  MotionSeg/gjsolver.o MotionSeg/affineModel.o -lgsl -lgslcblas ${CXXLIBS}

lucaskanade: lucaskanade.o lkt.o ../Common/timer.o
	g++ -o lucaskanade -ggdb lkt.o ../Common/timer.o lucaskanade.o -L ../Common/ -L /usr/lib/ -lm ${CXXLIBS}

goodfeatures: goodfeatures.o
	g++ -o goodfeatures -ggdb goodfeatures.o -L /usr/lib/ -lm ${CXXLIBS}

floodFillSegment: floodFillSegment.o
	g++ -o floodFillSegment -ggdb floodFillSegment.o -L /usr/lib/ -lm ${CXXLIBS}

floodFillSegment1Chan: floodFillSegment1Chan.o
	g++ -o floodFillSegment1Chan -ggdb floodFillSegment1Chan.o -L /usr/lib/ -lm ${CXXLIBS}

colorThresh: colorThresh.o
	g++ -o colorThresh -ggdb colorThresh.o -L /usr/lib/ -lm ${CXXLIBS}

colorThresh1Chan: colorThresh1Chan.o
	g++ -o colorThresh1Chan -ggdb colorThresh1Chan.o -L /usr/lib/ -lm ${CXXLIBS}

normThresh: normThresh.o
	g++ -o normThresh -ggdb normThresh.o -L /usr/lib/ -lm ${CXXLIBS}

shapeTest: shapeTest.o
	g++ -o shapeTest -ggdb shapeTest.o -L /usr/lib/ -lm ${CXXLIBS}

checkerboard: checkerboard.o
	g++ -o checkerboard -ggdb checkerboard.o -L /usr/lib/ -lm ${CXXLIBS}

laserMouse: laserMouse.o
	g++ -o laserMouse -ggdb laserMouse.o -L /usr/lib/ -lm ${CXXLIBS}

keypoints: keypoints.o
	g++ -o keypoints -ggdb keypoints.o -L /usr/lib -lm ${CXXLIBS}

crosshairs: crosshairs.o
	g++ -o crosshairs -ggdb crosshairs.o -L /usr/lib -lm ${CXXLIBS}

motion: motion.o
	g++ -o motion -ggdb motion.o -L /usr/lib/ -lm ${CXXLIBS}

glcm: glcm.o
	g++ -o glcm -ggdb glcm.o -L /usr/lib -lm ${CXXLIBS}

glcm.o: glcm.cpp
	g++ -c glcm.cpp -ggdb

clean:
	rm -f *.o
	rm -f MotionSeg/*.o
	rm -f MotionSeg/motionseg
	rm -f MotionSeg/gjsolver
	rm -f motion
	rm -f glcm
	rm -f lucaskanade
	rm -f crosshairs
	rm -f keypoints
	rm -f floodFillSegment1Chan
	rm -f colorThresh1Chan
	rm -f blurEdges
	rm -f houghLines
	rm -f perspective
