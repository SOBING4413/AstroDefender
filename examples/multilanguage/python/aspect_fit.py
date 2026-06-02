# Python + pygame-ce: scale a logical Surface without stretching its aspect ratio.
import pygame

LOGICAL_SIZE = (960, 720)
BG = (0, 4, 18)

def aspect_fit_rect(window_size: tuple[int, int]) -> pygame.Rect:
    scale = min(window_size[0] / LOGICAL_SIZE[0], window_size[1] / LOGICAL_SIZE[1])
    size = (round(LOGICAL_SIZE[0] * scale), round(LOGICAL_SIZE[1] * scale))
    return pygame.Rect(((window_size[0] - size[0]) // 2, (window_size[1] - size[1]) // 2), size)

def present(window: pygame.Surface, logical: pygame.Surface) -> None:
    dst = aspect_fit_rect(window.get_size())
    window.fill(BG)
    window.blit(pygame.transform.smoothscale(logical, dst.size), dst)
    pygame.display.flip()
