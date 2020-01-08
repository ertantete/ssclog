# This is to automate builds of ssclog.exe (windows exe)
#
# Build this container:
# docker build . -t fizux/ssclog-builder
#
# Once container is created, run it:
# docker run -it -v $PWD:/out fizux/ssclog-builder
#
# the exe file should appear in $PWD (or, more likely, you will need to troubleshoot why it doesn't)


FROM ubuntu:18.04
LABEL maintainer="chris.norris.esq@gmail.com"

VOLUME /out
COPY . /source
RUN apt-get update && apt-get install build-essential gcc-mingw-w64 -y && apt-get autoclean

WORKDIR /source/scripts
CMD ./docker-run.sh
