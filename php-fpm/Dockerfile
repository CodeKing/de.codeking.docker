FROM phpdockerio/php71-fpm:latest
WORKDIR "/docker"

# fix debconf warnings upon build
ARG DEBIAN_FRONTEND=noninteractive

# install php extensions
RUN apt-get update \
    && apt-get -y --no-install-recommends install git php-memcached php7.1-mysql php-redis php-xdebug \
        php7.1-bcmath php7.1-bz2 imagemagick php-imagick php7.1-intl php7.1-phpdbg php-ssh2 php7.1-xmlrpc php7.1-opcache php7.1-zip \
        php7.1-mcrypt php7.1-mbstring php7.1-gd php7.1-json

# cleanup container
RUN apt-get clean; rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* /usr/share/doc/*

# run console shortcut
RUN echo "alias shop='php -q /docker/console.php'" >> /root/.bashrc
RUN echo "alias k='php -q /docker/console.php'" >> /root/.bashrc
RUN echo "alias kursolino='php -q /docker/console.php'" >> /root/.bashrc