add_executable ("example1" "example1.cc")
add_executable ("example2" "example2.cc")

add_executable ("play-sine-wave" "play-sine-wave.cc")
target_link_libraries ("play-sine-wave" dbus)
target_link_libraries ("play-sine-wave" stats)
target_link_libraries ("play-sine-wave" audio)
target_link_libraries ("play-sine-wave" util)

target_link_libraries ("play-sine-wave" ${ALSA_LDFLAGS})
target_link_libraries ("play-sine-wave" ${ALSA_LDFLAGS_OTHER})

target_link_libraries ("play-sine-wave" ${DBus_LDFLAGS})
target_link_libraries ("play-sine-wave" ${DBus_LDFLAGS_OTHER})
target_link_libraries ("play-sine-wave" ${Sndfile_LDFLAGS})
target_link_libraries ("play-sine-wave" ${Sndfile_LDFLAGS_OTHER})

add_executable ("play-wav-file" "play-wav-file.cc")
target_link_libraries ("play-wav-file" dbus)
target_link_libraries ("play-wav-file" stats)
target_link_libraries ("play-wav-file" audio)
target_link_libraries ("play-wav-file" util)

target_link_libraries ("play-wav-file" ${ALSA_LDFLAGS})
target_link_libraries ("play-wav-file" ${ALSA_LDFLAGS_OTHER})

target_link_libraries ("play-wav-file" ${DBus_LDFLAGS})
target_link_libraries ("play-wav-file" ${DBus_LDFLAGS_OTHER})
target_link_libraries ("play-wav-file" ${Sndfile_LDFLAGS})
target_link_libraries ("play-wav-file" ${Sndfile_LDFLAGS_OTHER})

target_link_libraries ("play-wav-file" ${Samplerate_LDFLAGS})
target_link_libraries ("play-wav-file" ${Samplerate_LDFLAGS_OTHER})

add_executable ("play-periods" "play-periods.cc")
target_link_libraries ("play-periods" dbus)
target_link_libraries ("play-periods" stats)
target_link_libraries ("play-periods" audio)
target_link_libraries ("play-periods" util)

target_link_libraries ("play-periods" ${ALSA_LDFLAGS})
target_link_libraries ("play-periods" ${ALSA_LDFLAGS_OTHER})

target_link_libraries ("play-periods" ${DBus_LDFLAGS})
target_link_libraries ("play-periods" ${DBus_LDFLAGS_OTHER})
target_link_libraries ("play-periods" ${Sndfile_LDFLAGS})
target_link_libraries ("play-periods" ${Sndfile_LDFLAGS_OTHER})

target_link_libraries ("play-periods" ${Samplerate_LDFLAGS})
target_link_libraries ("play-periods" ${Samplerate_LDFLAGS_OTHER})

add_executable ("transform-periods" "transform-periods.cc")
target_link_libraries ("transform-periods" dbus)
target_link_libraries ("transform-periods" stats)
target_link_libraries ("transform-periods" audio)
target_link_libraries ("transform-periods" signal)
target_link_libraries ("transform-periods" util)

target_link_libraries ("transform-periods" ${ALSA_LDFLAGS})
target_link_libraries ("transform-periods" ${ALSA_LDFLAGS_OTHER})

target_link_libraries ("transform-periods" ${DBus_LDFLAGS})
target_link_libraries ("transform-periods" ${DBus_LDFLAGS_OTHER})
target_link_libraries ("transform-periods" ${Sndfile_LDFLAGS})
target_link_libraries ("transform-periods" ${Sndfile_LDFLAGS_OTHER})

target_link_libraries ("transform-periods" ${Samplerate_LDFLAGS})
target_link_libraries ("transform-periods" ${Samplerate_LDFLAGS_OTHER})

target_link_libraries ("transform-periods" ${Boost_LDFLAGS})
target_link_libraries ("transform-periods" ${Boost_LDFLAGS_OTHER})
target_link_libraries ("transform-periods" ${FFTW_LDFLAGS})
target_link_libraries ("transform-periods" ${FFTW_LDFLAGS_OTHER})
target_link_libraries ("transform-periods" "-lfftw3_threads")

add_executable ("print-wav-file" "print-wav-file.cc")
target_link_libraries ("print-wav-file" dbus)
target_link_libraries ("print-wav-file" stats)
target_link_libraries ("print-wav-file" audio)
target_link_libraries ("print-wav-file" util)

target_link_libraries ("print-wav-file" ${Sndfile_LDFLAGS})
target_link_libraries ("print-wav-file" ${Sndfile_LDFLAGS_OTHER})

target_link_libraries ("print-wav-file" ${Samplerate_LDFLAGS})
target_link_libraries ("print-wav-file" ${Samplerate_LDFLAGS_OTHER})

add_executable ("midi-input" "midi-input.cc")
target_link_libraries ("midi-input" dbus)
target_link_libraries ("midi-input" stats)
target_link_libraries ("midi-input" audio)
target_link_libraries ("midi-input" util)

target_link_libraries ("midi-input" ${ALSA_LDFLAGS})
target_link_libraries ("midi-input" ${ALSA_LDFLAGS_OTHER})

target_link_libraries ("midi-input" ${DBus_LDFLAGS})
target_link_libraries ("midi-input" ${DBus_LDFLAGS_OTHER})
target_link_libraries ("midi-input" ${Sndfile_LDFLAGS})
target_link_libraries ("midi-input" ${Sndfile_LDFLAGS_OTHER})


add_executable ("synthesizer-test" "synthesizer-test.cc")
target_link_libraries ("synthesizer-test" dbus)
target_link_libraries ("synthesizer-test" stats)
target_link_libraries ("synthesizer-test" audio)
target_link_libraries ("synthesizer-test" util)

target_link_libraries ("synthesizer-test" ${ALSA_LDFLAGS})
target_link_libraries ("synthesizer-test" ${ALSA_LDFLAGS_OTHER})

target_link_libraries ("synthesizer-test" ${DBus_LDFLAGS})
target_link_libraries ("synthesizer-test" ${DBus_LDFLAGS_OTHER})
target_link_libraries ("synthesizer-test" ${Sndfile_LDFLAGS})
target_link_libraries ("synthesizer-test" ${Sndfile_LDFLAGS_OTHER})

target_link_libraries ("synthesizer-test" ${Samplerate_LDFLAGS})
target_link_libraries ("synthesizer-test" ${Samplerate_LDFLAGS_OTHER})

add_executable ("synth-worst-case" "synth-worst-case.cc")
target_link_libraries ("synth-worst-case" dbus)
target_link_libraries ("synth-worst-case" stats)
target_link_libraries ("synth-worst-case" audio)
target_link_libraries ("synth-worst-case" util)

target_link_libraries ("synth-worst-case" ${ALSA_LDFLAGS})
target_link_libraries ("synth-worst-case" ${ALSA_LDFLAGS_OTHER})

target_link_libraries ("synth-worst-case" ${DBus_LDFLAGS})
target_link_libraries ("synth-worst-case" ${DBus_LDFLAGS_OTHER})
target_link_libraries ("synth-worst-case" ${Sndfile_LDFLAGS})
target_link_libraries ("synth-worst-case" ${Sndfile_LDFLAGS_OTHER})

target_link_libraries ("synth-worst-case" ${Samplerate_LDFLAGS})
target_link_libraries ("synth-worst-case" ${Samplerate_LDFLAGS_OTHER})
