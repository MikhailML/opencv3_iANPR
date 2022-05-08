FROM ubuntu:16.04

# Setup Environment Variable
ENV cvVersionChoice=1
ENV cvVersion="3.4.6"
ENV cwd="/home/"

WORKDIR $cwd

RUN apt-get update && \
	apt-get remove -y \
	x264 libx264-dev && \
	apt-get install -y \
	build-essential \
	checkinstall \
	cmake \
	pkg-config \
	yasm \
	git \
	gfortran \
	libjpeg8-dev \
	libjasper-dev \
	libpng12-dev \
	libtiff5-dev \
	libtiff-dev \
	libavcodec-dev \
	libavformat-dev \
	libswscale-dev \
	libdc1394-22-dev \
	libxine2-dev \
	libv4l-dev

RUN cd /usr/include/linux && \
	ln -s -f ../libv4l1-videodev.h videodev.h && \
	cd $cwd

RUN apt-get install -y \
	libgstreamer0.10-dev \
	libgstreamer-plugins-base0.10-dev \
	libgtk2.0-dev \
	libtbb-dev \
	qt5-default \
	libatlas-base-dev \
	libfaac-dev \
	libmp3lame-dev \
	libtheora-dev \
	libvorbis-dev \
	libxvidcore-dev \
	libopencore-amrnb-dev \
	libopencore-amrwb-dev \
	libavresample-dev \
	x264 \
	v4l-utils \
	libprotobuf-dev \
	protobuf-compiler \
	libgoogle-glog-dev \
	libgflags-dev \
	libgphoto2-dev \
	libeigen3-dev \
	libhdf5-dev \
	doxygen

RUN apt-get install -y \
	python-dev \
	python-pip \
	python3-dev \
	python3-pip

RUN git clone https://github.com/opencv/opencv.git && \
	cd opencv && \
	git checkout $cvVersion && \
	cd ..

RUN git clone https://github.com/opencv/opencv_contrib.git && \
	cd opencv_contrib && \
	git checkout $cvVersion && \
	cd ..

RUN cd opencv && \
	mkdir build && \
	cd build && \
        cmake -D CMAKE_BUILD_TYPE=RELEASE \
        -D CMAKE_INSTALL_PREFIX=/usr/local \
        -D BUILD_opencv_world=ON \
        -D INSTALL_C_EXAMPLES=OFF \
        -D INSTALL_PYTHON_EXAMPLES=OFF \
        -D WITH_TBB=ON \
        -D WITH_V4L=ON \
        -D WITH_QT=ON \
        -D WITH_OPENGL=ON \
        -D OPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules \
        -D BUILD_EXAMPLES=OFF .. && \
        make -j2 && \
        make install

COPY libianpr1.8.so libianprinterface1.8.so libianprcapture1.8.so /usr/local/lib/
RUN /bin/sh -c 'echo "/usr/local/lib" >> /etc/ld.so.conf.d/opencv.conf'
RUN ldconfig
COPY samples/* ./

