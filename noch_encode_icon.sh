convert res/lagrange-256.png -resize 48x48 lagrange-48.png
optipng -strip all lagrange-48.png
uuencode -m lagrange-48.png lagrange-48.png \
| sed -e 's/^/ /' \
> lagrange-48.png.base64
echo "checking head:"
head lagrange-48.png.base64
echo "checking tail:"
tail lagrange-48.png.base64
