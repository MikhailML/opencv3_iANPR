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

#RUN pip2 install -U pip numpy && \
#	pip3 install -U pip numpy

#RUN apt-get install -y python3-testresources

#RUN python3 -m pip uninstall -y pip && \
#	apt install -y python3-pip --reinstall

#RUN python -m pip uninstall pip && \
#	apt install python-pip --reinstall

#RUN pip2 install -U virtualenv virtualenvwrapper && \
#	python3 -m pip install -U virtualenv virtualenvwrapper

#RUN python3 -m pip install -U jupyter jupyterhub==0.8.1 notebook

#RUN echo "# Virtual Environment Wrapper" >> ~/.bashrc && \
#	echo 'source /usr/local/bin/virtualenvwrapper.sh' >> ~/.bashrc && \
#	cd $cwd

#RUN /bin/bash -c "source /usr/local/bin/virtualenvwrapper.sh && \
#	mkvirtualenv OpenCV-\"$cvVersion\"-py2 -p python2 && \
#	workon OpenCV-\"$cvVersion\"-py2 && \
#	pip install numpy scipy matplotlib scikit-image scikit-learn ipython && \
#	pip install ipykernel && \
#	python -m ipykernel install --name OpenCV-$cvVersion-py2 && \
#	deactivate && \
#	mkvirtualenv OpenCV-\"$cvVersion\"-py3 -p python3 && \
#	workon OpenCV-\"$cvVersion\"-py3 && \
#	pip install numpy scipy matplotlib scikit-image scikit-learn ipython && \
#	pip install ipykernel && \
#	python -m ipykernel install --name OpenCV-$cvVersion-py3 && \
#	deactivate"

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

#WORKDIR /root/.virtualenvs/OpenCV-$cvVersion-py2/lib/python2.7/site-packages
#RUN py2binPath=$(find /usr/local/lib/ -type f -name "cv2.so") && \
#	ln -s -f py2binPath cv2.so

#WORKDIR /root/.virtualenvs/OpenCV-$cvVersion-py3/lib/python3.5/site-packages
#RUN py3binPath=$(find /usr/local/lib/ -type f -name "cv2.cpython*.so") && \
#	ln -s -f py3binPath cv2.so

#WORKDIR $cwd
#ENV TMPPATH="$PATH"
#ENV PATH="/root/anaconda3/bin:$PATH"
#RUN apt-get install wget && \
#	wget https://repo.anaconda.com/archive/Anaconda3-5.2.0-Linux-x86_64.sh && \
#	chmod u+x Anaconda3-5.2.0-Linux-x86_64.sh && \
#	/bin/bash -c "./Anaconda3-5.2.0-Linux-x86_64.sh -b && \
#	echo 'export PATH=\"/root/anaconda3/bin:$PATH\"' >> ~/.bashrc && \
#	source ~/.bashrc && \
#	/root/anaconda3/bin/conda install -y xeus-cling notebook -c QuantStack -c conda-forge && \
#	/root/anaconda3/bin/conda install -y -c conda-forge jupyterhub==0.8.1"

#RUN apt-get install -y wget && \
#	wget https://repo.anaconda.com/archive/Anaconda3-5.2.0-Linux-x86_64.sh && \
#	chmod u+x Anaconda3-5.2.0-Linux-x86_64.sh && \
#	/bin/bash -c "./Anaconda3-5.2.0-Linux-x86_64.sh -b && \
#	/root/anaconda3/bin/conda install -y xeus-cling notebook -c QuantStack -c conda-forge" && \
	#/root/anaconda3/bin/conda install -y -c conda-forge jupyterhub==0.8.1" && \
#	rm Anaconda3-5.2.0-Linux-x86_64.sh
#RUN cp -r ~/anaconda3/share/jupyter/kernels/xeus-cling-cpp1* /usr/local/share/jupyter/kernels/
#RUN apt-get install -y vim
#ENV PATH=$TMPPATH
#RUN apt-get install -y nodejs \
#	npm && \
#	npm install -g configurable-http-proxy && \
#	ln -s /usr/bin/nodejs /usr/bin/node

#ENV DEBIAN_FRONTEND teletype
