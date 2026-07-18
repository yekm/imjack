
set -e
export rec=$1
export trec=/tmp/$rec
#export parallel='parallel --tag -k -j1 --halt now,fail=1'
export parallel='parallel --tag -k --lb' # --halt now,fail=1'

echo "$(date +%F_R) $WAV $rec" >> log.txt

# ffmpeg -y -hide_banner -loglevel info -channels 1 -f jack -i ffmpeg_wav -channels 1 -f wav -c copy mic1.wav
# sox sine400-sweep.wav -t wav - gain -30 | aplay -

WAV=$2
[ -z "$WAV" ] && exit -1
# record $rec-l-r.wav
sox $WAV -c1 $rec-l.wav remix 1 trim 00:04.20 00:60
sox $WAV -c1 $rec-r.wav remix 1 trim 00:66.20 00:60
parallel 'sox {} -n spectrogram -x 3920 -Y 1080 -o {}.png' ::: $rec-?.wav
#exit
$parallel "sox $rec-{}.wav -t f32 -r 48000 -c 1 $trec-{}.pcm" ::: r l
$parallel "drc-3.2.3/source/lsconv $trec-{}.pcm inverse.pcm $trec-impulse-{}.pcm" ::: l r


#export drc_args="--PLMaxGain=1.2 --MSNormType=E --PSNormFactor=1"
#drc_args="$drc_args --PSNormType=S --PSNormFactor=16"
#drc_args="$drc_args --PSNormType=E --PSNormFactor=1"

# from normal-48.drc:
window=$(python -c "print(int(48000*0.3))")
#drc_args="$drc_args --MPLowerWindow=48000 --EPLowerWindow=2044 --RTLowerWindow=48000 --ISPELowerWindow=1022"
#drc_args="$drc_args --MPLowerWindow=$window --EPLowerWindow=2044 --RTLowerWindow=$window --ISPELowerWindow=1022"
#drc_args="$drc_args --MPLowerWindow=35520 --EPLowerWindow=1488"
export drc_args

#export t=pa-48.0.txt
export target=bk-3-subultra-spline-48.0.txt

export len=8
export len1=7
# convolve FOR $1 base .drc
conv4() {
	drc=soft
	# MPLowerWindow 16384 ... 65536
	# MPUpperWindow 22 ... 128 
	i=$1
	j=$2
	#we=$2
	fo=filter/filter-$rec-$drc-$i-$j

	#n=$(( $i * $len + $j ))
	n=$(( $j ))
	ch1=$(( $n * 2 + 1 ))
	ch2=$(( $n * 2 + 2 ))
	#ch="out-$i-$j"
	
#/input/name      $ch1    $d-in-$s-1
#/input/name      $ch2    $d-in-$s-2
	d=win
	cat << EOF
/output/name     $ch1    out-$n-l
/output/name     $ch2    out-$n-r
/impulse/read    1   $ch1    1      0      0       0    1     $fo-l.wav
/impulse/read    2   $ch2    1      0      0       0    1     $fo-r.wav
EOF

    # min + (step_len) * n
     w=$(bc -l <<< "0.6 +   $i * (1.5   - 0.6)   / $len")
    lw=$(bc -l <<< "16384 + $j * (65536 - 16384) / $len" | cut -f1 -d.)
    uw=$(bc -l <<< "22 +    $j * (128   - 22)    / $len" | cut -f1 -d.)

	drc_args="$drc_args --MPWindowExponent=$w --EPWindowExponent=$w"
	#drc_args="$drc_args --MPLowerWindow=$lw --MPUpperWindow=$w"
	drc_args="$drc_args --MPLowerWindow=$lw"
	
	echo "$i $j $n $ch1 $ch2 $w $lw $uw" >> /tmp/drc.log

	#[ -s $fo-l.wav ] && exit
	#SKIP=1
	[ -n "$SKIP" ] && [ -s $fo-l.wav ] && exit
	o=$i-$j
	$parallel "./drc --BCInFile=$trec-{}.pcm --PSOutFile=$trec-filter-$o-{}.pcm --PSPointsFile='drc-3.2.3/source/target/48.0 kHz/'$target $drc_args 'drc-3.2.3/source/config/48.0 kHz/'$drc-48.0.drc" ::: l r |& tee $trec-$o-drc.log |& pv -Xq
	
	$parallel "sox -t f32 -r 48000 -c1 $trec-filter-$o-{}.pcm $fo-{}.wav" ::: l r |& pv -Xq
	sox $fo-?.wav --combine merge lsp/$fo-stereo.wav |& pv -Xq
}
convw() {
	jcc=$jc-$1.conf
	echo $jcc >>jc.conf.list
	cat > $jcc << EOF
#                        in  out   partition    maxsize
# ---------------------------------------------------------------
/convolver/new    2    $(( $len * 2))        1024        65536
#
/input/name      1    in-1
/input/name      2    in-2
#
#               in out  gain  delay  offset  length  chan      file
# --------------------------------------------------------------------------
EOF

    parallel -k conv4 $1 ::: $(seq 0 $len1) | tee -a $jcc
}
export -f convw conv4

export jc=jc-$rec

rm -f jc.conf.list
parallel -j2 -k convw ::: $(seq 0 $len1)
#exit

echo ok?
read qwe
ln -vsf $jc jc-service.conf
killall jconvolver || true

exit

#cat ~/etc/audio/drc/jc.conf.list |
#	parallel 'jconvolver -N {} ~/etc/audio/drc/{}'



#parallel "sox -t f32 -r 48000 -c1 $trec-filter-{}.pcm filter-{}.wav" ::: l r

##parallel 'sox {} -n spectrogram -x 1920 -Y 1080 -o {}.png' ::: *.wav

# The first parameters to modify are those that define the windowing correction curve applied to the signal, i.e MPWindowExponent, EPWindowExponent, RTWindowExponent, slowly reducing them to 0.95, 0.9, 0.85 and so on, down to about 0.7, thus reducing the correction in the critical mid and mid-bass range. These are really sensitive parameters, so changing them by as little as 0.01 easily cause an audible difference, especially when you are close to the boundary where correction artifacts start to appear.
# When the artifacts disappear you can start increasing the windows applied to the bass range, slowly increasing, by about a 5% at a time, the MPLowerWindow, EPLowerWindow, RTLowerWindow parameters, until artifacts start to appear again. After that you can decrease again the window exponent parameters until artifacts disappear again, and so on.
# This procedure may be repeated until there’s no further improvement or the parameters reach an excessive value, i.e below about 0.6 for the window exponents, above 1 second for the minimum phase and ringing truncation windowing parameters (MPLowerWindow, RTLowerWindow) and above 100 ms for the excess phase windowing parameter (EPLowerWindow). Remember also to set the pre-echo truncation parameters (ISPELowerWindow, ISPEUpperWindow) according to the excess phase windowing parameters (see sections 6.8.4 and 6.8.5).

# The sample configuration files supplied are a good example of all these options combined together. In normal situations you can use them as they are changing only EPLowerWindow, EPWindowExponent, MPLowerWindow and MPWindowExponent to fit your needs.



# 6.4.4  MPLowerWindow (*)
# Length of the window for the minimum phase component prefiltering at the bottom end of the frequency range. Longer windows cause DRC to try to correct a longer part of the impulse response but cause greater sensibility to the listening position. Typical values are between 16384 and 65536. MPLowerWindow must not be greater than BCInitWindow.

# 6.4.5  MPUpperWindow (*)
# Length of the window for the minimum phase component prefiltering at the upper end of the frequency range. Longer windows cause DRC to try to correct a longer part of the impulse response but cause greater sensibility on the listening position. Typical values are between 22 and 128. MPUpperWindow must be not greater than MPLowerWindow, and usually is much shorter than that.

# 6.4.8  MPWindowExponent (*)
# This is the exponent used in the frequency dependent window length computation for the band windowing procedure, or in the computation of the time dependent cutoff frequency for the sliding lowpass procedure.
# Changing the window exponent provides different prefiltering curves, see section 4.2 for a deeper explanation. Increasing the window exponent gives higher correction in the midrange. Typical values are between 0.7 and 1.2.

# 6.6.8  EPWindowExponent (*)
# Same as MPWindowExponent but for the excess phase component. See discussion on MPWindowExponent. Usual values for this parameter are between 0.5 and 1.2, depending on the value of EPInitWindow. As a rule of thumb you can take: EPWindowExponent = MPWindowExponent
