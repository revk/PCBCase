(define (index-png filename)
  (let* 
    (
    (image (car (gimp-file-load RUN-NONINTERACTIVE filename filename)))
    (drawable (car (gimp-image-get-active-layer image)))
    )
  (gimp-image-convert-indexed image NO-DITHER MAKE-PALETTE 0 0 1 "")
  (file-png-save RUN-NONINTERACTIVE image drawable filename filename 0 6 1 0 0 1 1)
  (gimp-image-delete image)
  )
)
