FROM phpdockerio/php71-fpm:latest
WORKDIR "/docker"

# fix debconf warnings upon build
ARG DEBIAN_FRONTEND=noninteractive

# install php extensions
RUN apt-get update \
    && apt-get -y --no-install-recommends install git php-memcached php7.1-mysql php-redis php-xdebug \
        php7.1-bcmath php7.1-bz2 imagemagick php-imagick php7.1-intl php7.1-phpdbg php-ssh2 php7.1-xmlrpc php7.1-opcache php7.1-zip \
        php7.1-mbstring php7.1-gd php7.1-json php7.1-imap php7.1-mcrypt xvfb cutycapt whois

# cleanup container
RUN apt-get clean; rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* /usr/share/doc/*

# install libsepa
RUN mkdir /usr/lib/php/libsepa/
COPY libsepa/Linux/64bit/php-7.1/sepa.so /usr/lib/php/libsepa/

# install sendmail
RUN curl -Lsf 'https://storage.googleapis.com/golang/go1.8.3.linux-amd64.tar.gz' | tar -C '/usr/local' -xvzf -
ENV PATH /usr/local/go/bin:$PATH
RUN go get github.com/mailhog/mhsendmail
RUN cp /root/go/bin/mhsendmail /usr/bin/mhsendmail

# run console shortcut
RUN echo "alias shop='php -q /docker/console.php'" >> ~/.bashrc
RUN echo "alias k='php -q /docker/console.php'" >> ~/.bashrc
RUN echo "alias kursolino='php -q /docker/console.php'" >> ~/.bashrc