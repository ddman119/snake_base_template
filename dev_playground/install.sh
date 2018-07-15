# file: install.sh
sudo apt-get update
sudo apt-get install python3
sudo apt-get install python3-dev
# install pygame
sudo apt-get install mercurial python3-dev python3-numpy ffmpeg \
libsdl-image1.2-dev libsdl-mixer1.2-dev libsdl-ttf2.0-dev
libsmpeg-dev \
libsdl1.2-dev libportmidi-dev libswscale-dev libavformat-dev
libavcodec-dev \
python-numpy subversion
(cd /usr/local/lib/python3.4/dist-packages/ && sudo svn co svn://seul.org/svn/pygame/trunk pygame)
(cd pygame && sudo python3 setup.py build && sudo python3 setup.py install)
# install swig
sudo apt-get install swig3.0
