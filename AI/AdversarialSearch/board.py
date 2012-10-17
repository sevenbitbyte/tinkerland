def draw_chessboard(n=8, pixel_width=200):
    "Draw an n x n chessboard using PIL."
    import Image, ImageDraw
    from itertools import cycle
    def sq_start(i):
        "Return the x/y start coord of the square at column/row i."
        return i * pixel_width / n
    
    def square(i, j):
        "Return the square corners, suitable for use in PIL drawings" 
        return map(sq_start, [i, j, i + 1, j + 1])
    
    image = Image.new("L", (pixel_width, pixel_width))
    draw_square = ImageDraw.Draw(image).rectangle
    squares = (square(i, j)
               for i_start, j in zip(cycle((0, 1)), range(n))
               for i in range(i_start, n, 2))
    for sq in squares:
        draw_square(sq, fill='white')
    image.save("chessboard-pil.png")

draw_chessboard()
