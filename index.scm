(define (index-png filename)
  (let* 
    (
    (image (car (gimp-file-load RUN-NONINTERACTIVE filename filename)))
    (drawable (car (gimp-image-get-active-layer image)))
    )
  (gimp-image-convert-indexed image 0 3 0 0 0 "")
  (file-png-save RUN-NONINTERACTIVE image drawable filename filename 0 9 0 0 0 0 0)
  (gimp-image-delete image)
  )
)
