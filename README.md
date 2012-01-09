Installation
=============

> $ ./configure
>
> $ make
>
> $ sudo make install


Requirements
=============

1. jack
2. mplayer


Usage
============

## run jack e.g to listen to netmanaget multicast sync

> $ jackd -dnet &

## run jmusync

> $ jmusync > /dev/null &
 
## run mplayer

> $ mplayer -udp-slave your-video.xxx

## on master (or localhost if no netmamanger active) start playback

> $ jack_transport
>
> jack_transport> stop
>
> jack_transport> locate 0
>
> jack_transport> play

If all went well mplayer should begin playing.

Remember mplayer cannot seek to non-keyframes, so best to use mencoder to re-encode files with keyint=1 to make every frame a key frame.

