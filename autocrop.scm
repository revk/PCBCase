(define (crop-png filename)
  (let* 
    (
    (image (car (gimp-file-load RUN-NONINTERACTIVE filename filename)))
    (drawable (car (gimp-image-get-active-layer image)))
    )
  (plug-in-autocrop RUN-NONINTERACTIVE image drawable)
  (gimp-palette-set-background '(255 255 255))
  (file-png-save RUN-NONINTERACTIVE image drawable filename filename 0 6 1 0 0 1 1)
  (gimp-image-delete image)
  )
)
