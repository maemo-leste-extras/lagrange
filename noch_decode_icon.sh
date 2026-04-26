awk '
  /^XB-Maemo-Icon-26:/ {in_icon=1; print "begin-base64 644 icon.png"; next}
  in_icon && /^ / {print substr($0,2); next}
  in_icon && !/^ / {print "===="; exit}
' debian/control > icon.uue

uudecode icon.uue
file icon.png

