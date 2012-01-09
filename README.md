Installation
=============

> $ ./configure
> 
> $ make
> 
> $ sudo make install"


Requirements
=============

jack
mplayer


Usage
============

1. run jack e.g to listen to netmanaget multicast sync

> $ jackd -dnet &

2. run jmusync

> $ jmusync > /dev/null &

3. run mplayer

> $ mplayer -udp-slave your-video.xxx

4. on master (or localhost if no netmamanger active) start playback

> "$ jack_transport
> 
> jack_transport> stop
> 
> jack_transport> locate 0
> 
> jack_transport> play

If all went well mplayer should begin playing.

Remember mplayer cannot seek to non-keyframes, so best to use mencoder to re-encode files with keyint=1 to make every frame a key frame.

