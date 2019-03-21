FROM ubuntu14lts

# Install Nginx
RUN apt-get install -y nginx

RUN echo "\ndaemon off;" >> /etc/nginx/nginx.conf

# set the working dir
WORKDIR /etc/nginx

#define the default command
ENTRYPOINT ["nginx"] 
