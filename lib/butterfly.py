
from ctypes import *

_library = None

_possible_library_names = [
    'butterfly.dll',        # Generic Windows DLL
    './libbutterfly.so',    # Local Linux SO
    './libbutterfly.dylib', # Local OS X dylib
    './butterfly.so',       # Local Linux SO w/o prefix
    'libbutterfly.so',      # System Linux SO
    'libbutterfly.dylib',   # System OS X dylib
    'butterflyso'           # System Linux SO w/o prefix
]
for name in _possible_library_names:
    try:
        _library = CDLL(name)
        break
    except OSError:
        continue

if _library is None:
    raise RuntimeError("butterfly library cannot be loaded.")

NARGS = 5
MAX_QUERY_RET = 2

class _bf_farm(Structure):
    _fields_ = [("spots", POINTER(c_int)),
                ("width", c_int),
                ("height", c_int),
                ("seed", c_int),
                ("last_dangerous", c_int),

                ("is_init", c_int),
                ("rng_state", c_void_p),
                ("safe_spots", c_void_p),
                ("dangerous_spots", c_void_p),
                ("butterfly", c_void_p),
                ("error", c_int)]

class _bf_instinct(Structure):
    _fields_ = [("action", c_int),
                ("args", c_int * NARGS)]

class _bf_config(Structure):
    _fields_ = [("error_on_looking_at_safe", c_int),
                ("error_on_looking_at_safe_neighbor_4p", c_int),
                ("error_on_looking_at_safe_neighbor_4x", c_int),
                ("error_on_looking_at_safe_neighbor_8", c_int),
                ("error_on_looking_outside_farm", c_int),
                ("look_method", c_int),
                ("enable_neighbor_look_8", c_int),
                ("neighbor_look_8", c_int),
                ("enable_neighbor_look_4", c_int),
                ("neighbor_look_4", c_int)]

_library.bf_spawn.argtypes = [POINTER(_bf_farm), POINTER(_bf_instinct), c_size_t, POINTER(_bf_config)]
_library.bf_spawn.restype = c_int
_library.bf_commit.argtypes = [POINTER(_bf_farm)]
_library.bf_random.argtypes = [POINTER(_bf_farm)]
_library.bf_random.restype = c_double
_library.bf_query.argtypes = [POINTER(_bf_farm), c_int, POINTER(c_int)]
_library.bf_cleanup.argtypes = [POINTER(_bf_farm)]

MORPH_AT_LAST_DEATH_SPOT,\
MORPH_AT_LAST_MORPH_SPOT,\
MORPH_AT_FIXED_SPOT,\
MORPH_AT_RANDOM_SPOT,\
MORPH_AT_RANDOM_SAFE_SPOT,\
MORPH_AT_RANDOM_DANGEROUS_SPOT,\
MORPH_AT_RANDOM_WEST_SPOT,\
MORPH_AT_RANDOM_EAST_SPOT,\
MORPH_AT_RANDOM_NORTH_SPOT,\
MORPH_AT_RANDOM_SOUTH_SPOT,\
MORPH_AT_RANDOM_WEST_EDGE_SPOT,\
MORPH_AT_RANDOM_EAST_EDGE_SPOT,\
MORPH_AT_RANDOM_NORTH_EDGE_SPOT,\
MORPH_AT_RANDOM_SOUTH_EDGE_SPOT,\
MORPH_AT_RANDOM_WESTEAST_EDGE_SPOT,\
MORPH_AT_RANDOM_NORTHSOUTH_EDGE_SPOT,\
MORPH_AT_RANDOM_EDGE_SPOT,\
GOAL_FIXED_SPOT,\
GOAL_RANDOM_SPOT,\
GOAL_RANDOM_SAFE_SPOT,\
GOAL_RANDOM_DANGEROUS_SPOT,\
GOAL_RANDOM_WEST_SPOT,\
GOAL_RANDOM_EAST_SPOT,\
GOAL_RANDOM_NORTH_SPOT,\
GOAL_RANDOM_SOUTH_SPOT,\
GOAL_RANDOM_WEST_EDGE_SPOT,\
GOAL_RANDOM_EAST_EDGE_SPOT,\
GOAL_RANDOM_NORTH_EDGE_SPOT,\
GOAL_RANDOM_SOUTH_EDGE_SPOT,\
GOAL_RANDOM_WESTEAST_EDGE_SPOT,\
GOAL_RANDOM_NORTHSOUTH_EDGE_SPOT,\
GOAL_RANDOM_EDGE_SPOT,\
FLUTTER_STILL,\
FLUTTER_WEIGHTED_4,\
FLUTTER_WEIGHTED_8,\
FLUTTER_TUNNEL,\
FLUTTER_LINE,\
LOOK_NOWHERE,\
LOOK_EVERYWHERE,\
LOOK_1_AREA,\
LOOK_PLUS_AREA,\
LOOK_BIG_PLUS_AREA,\
LOOK_X_AREA,\
LOOK_BIG_X_AREA,\
LOOK_RECT_AREA,\
LOOK_SHRINKING_RECT_AREA,\
LOOK_CIRCLE_AREA,\
LOOK_SHRINKING_CIRCLE_AREA,\
LOOK_DIAMOND_AREA,\
LOOK_SHRINKING_DIAMOND_AREA,\
DIE_AT_FIXED_SPOT,\
DIE_AT_SAFE_SPOT,\
DIE_AT_DANGEROUS_SPOT,\
DIE_AT_GOAL,\
DIE_AFTER_N,\
DIE_ONE_IN = range(56)

METHOD_RANDOM,\
METHOD_CYCLE,\
METHOD_PICK = range(3)

QUERY_SAFE_PERCENTAGE,\
QUERY_DANGEROUS_PERCENTAGE,\
QUERY_RANDOM_SAFE_SPOT,\
QUERY_RANDOM_DANGEROUS_SPOT,\
QUERY_LAST_DEATH_SPOT,\
QUERY_LAST_MORPH_SPOT = range(4)

ERROR_NONE,\
ERROR_CANCEL,\
ERROR_NO_MEM = range(3)

class Butterfly(object):

    def __init__(self, *instincts, **config):
        self._instincts = []
        self._config = _bf_config()
        self.set_instincts(instincts)
        self.set_opts(**config)

    def set_instincts(self, instincts):
        for i in instincts:
            if len(i) == 1:
                self.add_instinct(i[0])
            else:
                self.add_instinct(i[0], i[1])

    def add_instinct(self, action, args=[]):
        instinct = _bf_instinct(action, (c_int * NARGS)(*args))
        self._instincts.append(instinct)

    def clr_instincts(self):
        self._instincts = []

    def set_opts(self, **kwargs):
        for k, v in kwargs.items():
            setattr(self._config, k, v)

class Farm(object):

    def __init__(self, width, height, spots=None, seed=None):
        self._farm = _bf_farm()
        spots = spots or [0 for x in xrange(width * height)]
        self._farm.spots = (c_int * (width * height))(*spots)
        self._farm.width = width
        self._farm.height = height
        self._farm.seed = seed or 0
        self._farm.last_dangerous = 0
        self._farm.is_init = 0

    def last_dangerous(self, opt=None):
        if not opt:
            return self._farm.last_dangerous
        self._farm.last_dangerous = opt

    def spawn(self, bfs, ntimes=1, commit=False):
        if not isinstance(bfs, list):
            bfs = [bfs]
        ret = None
        for i in xrange(ntimes):
            for bf in bfs:
                length = len(bf._instincts)
                if length == 0:
                    break
                instincts = (_bf_instinct * length)(*bf._instincts)
                ret = _library.bf_spawn(self._farm, instincts, length, bf._config)
                # if we are not commiting then an error would reset bf
                if not commit and ret != ERROR_NONE:
                    break
            else:
                # if the inner loop exited normally we commit when there
                # was no error
                if commit and ret == ERROR_NONE:
                    self.commit()
                continue
            break
        return ret

    def commit(self):
        _library.bf_commit(self._farm)

    def random(self):
        return _library.bf_random(self._farm)

    def query(self, what):
        ret = (c_int * MAX_QUERY_RET)()
        _library.bf_query(self._farm, what, ret)
        return ret

    def cleanup(self):
        _library.bf_cleanup(self._farm)

    def width(self):
        return self._farm.width

    def height(self):
        return self._farm.height

    def seed(self):
        return self._farm.seed

    def spot_at(self, x, y):
        return self._farm.spots[y * self._farm.width + x]

    def safe_percentage(self):
        p = self.query(QUERY_SAFE_PERCENTAGE)
        return p[0] / 100.

def random_1x1(tile):
    return Butterfly(*[
        [MORPH_AT_RANDOM_SPOT],
        [LOOK_1_AREA, [tile]],
        [DIE_AFTER_N, [1]]
    ])

def random_nxm(tile, n, m):
    return Butterfly(*[
        [MORPH_AT_RANDOM_SPOT],
        [LOOK_RECT_AREA, [tile, n, m]],
        [DIE_AFTER_N, [1]]
    ])
