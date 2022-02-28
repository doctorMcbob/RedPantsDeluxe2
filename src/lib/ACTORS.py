ACTORS = {'plat1': {'name': 'PLAT', 'POS': (-64, 64), 'DIM': (512, 64), 'spriteoffset': [0, 0], 'sprites': {'PLATFORM00': 'platform000', 'PLATFORM01': 'platform001', 'PLATFORM02': 'platform002', 'PLATFORM10': 'platform010', 'PLATFORM11': 'platform011', 'PLATFORM12': 'platform012', 'PLATFORM20': 'platform020', 'PLATFORM21': 'platform021', 'PLATFORM22': 'platform022', 'TOUCHED00': 'touched000', 'TOUCHED01': 'touched001', 'TOUCHED02': 'touched002', 'TOUCHED10': 'touched010', 'TOUCHED11': 'touched011', 'TOUCHED12': 'touched012', 'TOUCHED20': 'touched020', 'TOUCHED21': 'touched021', 'TOUCHED22': 'touched022'}, 'scripts': {'START:0': ['', 'set self state PLATFORM', 'set self platform 1', 'set self frame 0', 'set self touched 0', ''], 'PLATFORM:0': ['', 'if self.touched > 0', '    set self state TOUCHED', 'endif', 'set self frame 0', ''], 'TOUCHED:0': ['', 'set self frame 0']}, 'tangible': True, 'plat1': 'plat1'}, 'plat2': {'name': 'PLAT', 'POS': (288, 64), 'DIM': (544, 64), 'spriteoffset': [0, 0], 'sprites': {'PLATFORM00': 'platform000', 'PLATFORM01': 'platform001', 'PLATFORM02': 'platform002', 'PLATFORM10': 'platform010', 'PLATFORM11': 'platform011', 'PLATFORM12': 'platform012', 'PLATFORM20': 'platform020', 'PLATFORM21': 'platform021', 'PLATFORM22': 'platform022', 'TOUCHED00': 'touched000', 'TOUCHED01': 'touched001', 'TOUCHED02': 'touched002', 'TOUCHED10': 'touched010', 'TOUCHED11': 'touched011', 'TOUCHED12': 'touched012', 'TOUCHED20': 'touched020', 'TOUCHED21': 'touched021', 'TOUCHED22': 'touched022'}, 'scripts': {'START:0': ['', 'set self state PLATFORM', 'set self platform 1', 'set self frame 0', 'set self touched 0', ''], 'PLATFORM:0': ['', 'if self.touched > 0', '    set self state TOUCHED', 'endif', 'set self frame 0', ''], 'TOUCHED:0': ['', 'set self frame 0']}, 'tangible': True, 'plat2': 'plat2'}, 'player': {'name': 'player', 'POS': (32, 0), 'DIM': (32, 64), 'spriteoffset': [-16, 0], 'sprites': {'START:0': 'redpantsstand', 'IDLE:0': 'redpantsstand', 'RUNNING:0': 'redpantsrun0', 'RUNNING:4': 'redpantsrun1', 'RUNNING:8': 'redpantsrun2', 'RUNNING:12': 'redpantsrun3', 'AIRIAL:0': 'redpantsjump1', 'SLIDE:0': 'redpantsslide0', 'LANDING:0': 'redpantsland', 'JUMPSQUAT:0': 'redpantsjumpsquat0', 'JUMPSQUAT:3': 'redpantsjumpsquat1', 'FLIPSQUAT:0': 'redpantsjumpsquat0', 'FLIPSQUAT:3': 'redpantsjumpsquat1', 'DIVESTART:0': 'redpantsdivestart', 'DIVE:0': 'redpantsdive', 'DIVELAND:0': 'redpantsdiveland', 'ROLLOUT:0': 'redpantsrollout', 'BONK:0': 'redpantsbonk', 'BONKLAND:0': 'redpantsbonkland0', 'BONKLAND:12': 'redpantsbonkland1', 'WALLTOUCH:0': 'redpantswalljump0', 'WALLJUMP:0': 'redpantswalljump1', 'WALLJUMP:3': 'redpantswalljump2', 'SLIDEFLIP:0': 'redpantsdiveflip0', 'SLIDEFLIP:8': 'redpantsdiveflip1', 'SLIDEFLIP:10': 'redpantsdiveflip2', 'SLIDEFLIP:14': 'redpantsdiveflip3', 'SLIDEFLIP:18': 'redpantsdiveflip4', 'SLIDEFLIP:20': 'redpantsdiveflip5', 'SLIDEFLIP:23': 'redpantsdiveflip6', 'CROUCH:0': 'redpantscrouch', 'LONGJUMPSTART:0': 'redpantslongjumpstart', 'LONGJUMP:0': 'redpantslongjump1', 'LONGJUMP:8': 'redpantslongjump2'}, 'scripts': {'START:0': ['', '# welcome to red pants script :)', 'set self _input_name PLAYER1', 'set self state IDLE', 'set self frame 0', 'set self speed 8', 'set self jumpstrength -17', 'set self hopstrength -12', 'set self divestrength 14', 'set self rolloutstrength -8', 'set self bonkstrength 2', 'set self walljumpstrength -12', 'set self walljumpoff 10', 'set self airdrift 0.7', 'set self traction 0.6 * -1', 'set self limit 20', 'set self airspeedcontrol 12', 'set self negairspeedcontrol self.airspeedcontrol * -1', 'set self negspeed self.speed * -1', 'set self neglimit self.limit * -1', 'set self grav 1', 'set self slideflipxmod 3', 'set self slideflipstrength -24', 'set self longjumpystrength -9', 'set self longjumpxstrength 12', 'focus MAIN self', 'set self frame_name MAIN', ''], 'IDLE:0': ['', 'exec checkAir', 'if self.x_vel != 0', '   set self state SLIDE', 'endif', 'if self.y_vel == 0', '   if not inpRIGHT and inpLEFT', '      set self state RUNNING', '      set self frame 0', '      set self direction -1', '   endif', '   if not inpLEFT and inpRIGHT', '      set self state RUNNING', '      set self frame 0', '      set self direction 1', '   endif', 'endif', 'exec checkCrouch', 'exec checkJump', 'exec applyGrav', ''], 'IDLE:32': ['', 'exec IDLE:0', 'set self frame 0', ''], 'CROUCH:0': ['', 'exec checkAir', 'exec applyTraction', 'exec applyGrav', 'if not inpDOWN', '   set self state IDLE', '   set self frame 0', 'endif', '# bufferable pog?', 'if self.x_vel != 0 and inpA', '   set self state LONGJUMPSTART', '   set self frame 0', 'endif', 'set self frame 0', ''], 'LONGJUMPSTART:0': ['', 'exec checkAir', 'exec applyTraction', 'exec applyGrav', ''], 'LONGJUMPSTART:2': ['', 'exec LONGJUMPSTART:0', 'set self y_vel self.longjumpystrength', 'set self x_vel self.longjumpxstrength * self.direction', 'set self state LONGJUMP', 'set self frame 0', ''], 'LONGJUMP:0': ['', 'exec applyGrav', 'set self calc1 self.x_vel < self.negairspeedcontrol', 'if not self.calc1 and inpLEFT', '   set self x_vel self.x_vel - self.airdrift', 'endif', 'set self calc1 self.x_vel > self.airspeedcontrol', 'if not self.calc1 and inpRIGHT', '   set self x_vel self.x_vel + self.airdrift', 'endif', ''], 'RUNNING:0': ['', 'set self x_vel self.speed * self.direction', 'set self calc1 self.direction == 1 nor inpRIGHT', 'set self calc2 self.direction == -1 nor inpLEFT', 'if inpRIGHT == inpLEFT or self.calc1 or self.calc2', '   set self state SLIDE', '   set self frame 0', 'endif', 'exec checkCrouch', 'exec checkJump', 'exec checkAir', 'exec applyGrav', ''], 'RUNNING:16': ['', 'exec RUNNING:0', 'set self frame 0', ''], 'SLIDE:0': ['', 'if self.x_vel == 0', '   set self state IDLE', '   set self frame 0', 'endif', 'if self.x_vel != 0', '   exec applyTraction', '   set self absxvel abs self.x_vel', '   if abs self.traction * 10 > self.absxvel', '      img redpantsslide1', '   endif', '   exec checkCrouch', '   exec checkJump', '   if self.state == JUMPSQUAT', '      if self.direction == 1 and inpLEFT', '      \t set self state FLIPSQUAT', '\t set self frame 0', '      endif', '      if self.direction == -1 and inpRIGHT', '      \t set self state FLIPSQUAT', '\t set self frame 0', '      endif', '   endif', '   exec checkAir', '   exec applyGrav', 'endif', 'if abs self.x_vel < self.speed', '   set self calc1 self.x_vel > 0 and inpRIGHT', '   set self calc1 not inpLEFT and self.calc1', '   if self.calc1', '      set self direction 1', '   endif', '   if not inpRIGHT and self.x_vel < 0 and inpLEFT', '      set self direction -1', '   endif', '   if not inpRIGHT and self.x_vel < 0 and inpLEFT or self.calc1', '      set self state RUNNING', '      set self frame 0', '   endif', 'endif', ''], 'AIRIAL:0': ['', 'set self calc1 self.y_vel > self.airspeedcontrol', 'set self calc2 self.airspeedcontrol * -1 > self.y_vel', 'if not self.calc1 nor self.calc2', '   if inpLEFT', '\tset self x_vel self.x_vel - self.airdrift', '   \tif self.x_vel < self.negspeed', '      \t   set self x_vel self.negspeed', '   \tendif', '   endif', '   if inpRIGHT', '      set self x_vel self.x_vel + self.airdrift', '      if self.x_vel > self.speed', '      \t set self x_vel self.speed', '      endif', '   endif', '   if B_DOWN in inpEVENTS', '      set self state DIVESTART', '      set self frame 0', '   endif', 'endif', 'if self.calc1', '   img redpantsjump2', 'endif', 'if self.calc2', '   img redpantsjump0', 'endif', 'exec applyGrav', ''], 'JUMPSQUAT:0': ['', 'exec checkAir', 'exec applyTraction', 'exec applyGrav', ''], 'JUMPSQUAT:6': ['', 'set self state AIRIAL', 'if inpA', '   set self y_vel self.jumpstrength', 'endif', 'if not inpA', '   set self y_vel self.hopstrength', 'endif', 'set self frame 0', ''], 'LANDING:0': ['', 'exec checkAir', 'exec applyTraction', 'exec applyGrav', 'if self.x_vel > 0', '   set self direction 1', 'endif', 'if self.x_vel < 0', '   set self direction -1', 'endif', ''], 'LANDING:5': ['', 'if self.x_vel', '   set self state SLIDE', '   exec SLIDE:0', 'endif', 'if self.x_vel == 0', '   set self state IDLE', '   exec IDLE:0', 'endif', ''], 'DIVESTART:0': ['', 'set self y_vel 0', 'set self x_vel 0', ''], 'DIVESTART:5': ['', 'set self x_vel self.divestrength * self.direction', 'set self state DIVE', ''], 'DIVE:0': ['', 'exec applyGrav', ''], 'DIVELAND:0': ['', 'exec checkAir', 'exec applyTraction', 'exec applyGrav', '# bufferable Pog?', 'if inpA', '   set self state ROLLOUT', '   set self frame 0', '   set self y_vel self.rolloutstrength', 'endif', 'if self.x_vel == 0', '   set self state IDLE', '   set self frame 0', 'endif', ''], 'ROLLOUT:0': ['', 'exec applyGrav', ''], 'BONK:0': ['', 'set self x_vel self.bonkstrength * self.direction * -1', 'exec applyGrav', ''], 'BONKLAND:0': ['', 'exec checkAir', 'exec applyTraction', 'exec applyGrav', ''], 'BONKLAND:25': ['', 'set self state IDLE', 'set self frame 0', ''], 'WALLTOUCH:0': ['', 'exec AIRIAL:0', 'set self state AIRIAL', 'if not self.calc1 nor self.calc2', '   img redpantswalljump0', '   # bufferable pog?', '   if inpA', '      set self state WALLJUMP', '      set self frame 0', '   endif', 'endif', ''], 'WALLJUMP:0': ['', 'set self y_vel 0', ''], 'WALLJUMP:6': ['', 'set self state AIRIAL', 'set self frame 0', 'set self y_vel self.walljumpstrength', 'set self direction self.direction * -1', 'set self x_vel self.walljumpoff * self.direction', ''], 'FLIPSQUAT:0': ['', 'exec checkAir', 'exec applyTraction', 'exec applyGrav', ''], 'FLIPSQUAT:6': ['', 'set self state SLIDEFLIP', 'set self direction self.direction * -1', 'set self x_vel self.slideflipxmod * self.direction', 'set self y_vel self.slideflipstrength', 'set self frame 0', ''], 'SLIDEFLIP:0': ['', 'exec applyGrav', 'if self.direction == -1', '   if inpLEFT', '      \tset self x_vel self.x_vel - self.airdrift', '\tif self.x_vel < self.negairspeedcontrol', '      \t   set self x_vel self.negairspeedcontrol', '   \tendif', '   endif', 'endif', 'if self.direction == 1', '   if inpRIGHT', '      set self x_vel self.x_vel + self.airdrift', '      if self.x_vel > self.airspeedcontrol', '      \t set self x_vel self.airspeedcontrol', '      endif', '   endif', 'endif', '', ''], 'XCOLLISION': ['', 'set self calc1 self.state == DIVE', 'set self calc1 self.state == DIVELAND or self.calc1', 'set self calc1 self.state == LONGJUMP or self.calc1', 'if self.state == ROLLOUT or self.calc1', '   set self state BONK', '   set self frame 0', 'endif', 'set self calc1 self.state == SLIDEFLIP', 'if self.state == AIRIAL or self.calc1', '   set self state WALLTOUCH', '   set self frame 0', '   if self.x_vel > 0', '      set self direction 1', '   endif', '   if self.x_vel < 0', '      set self direction -1', '   endif', 'endif', ''], 'YCOLLISION': ['', 'set self calc1 self.state == ROLLOUT', 'set self calc1 self.state == SLIDEFLIP or self.calc1', 'set self calc1 self.state == LONGJUMP or self.calc1', 'if self.state == AIRIAL or self.calc1', '   set self state LANDING', '   set self frame 0', 'endif', 'if self.state == DIVE', '   set self state DIVELAND', '   set self frame 0', 'endif', 'if self.state == BONK', '   set self state BONKLAND', '   set self frame 0', 'endif', ''], 'COLLIDE': ['', 'if self.state == PLATFORM', '   set self touched 1', 'endif', ''], 'checkAir': ['', 'if self.y_vel', '   set self state AIRIAL', '   set self frame 0', 'endif', ''], 'applyGrav': ['', 'set self y_vel self.y_vel + self.grav', 'if self.y_vel > self.limit', '   set self y_vel self.limit', 'endif', 'if self.y_vel < self.neglimit', '   set self y_vel self.neglimit', 'endif', ''], 'applyTraction': ['', 'if self.x_vel != 0', '   if self.x_vel > 0', '      set self x_vel self.traction + self.x_vel', '   endif', '   if self.x_vel < 0', '      set self x_vel self.traction * -1 + self.x_vel', '   endif', 'endif', 'if abs self.x_vel / 1 == 0', '   set self x_vel 0', 'endif', ''], 'checkCrouch': ['', 'if inpDOWN', '   set self state CROUCH', '   set self frame 0', 'endif', ''], 'checkJump': ['', 'if A_DOWN in inpEVENTS', '   set self state JUMPSQUAT', '   set self frame 0', 'endif', '']}, 'tangible': True}, 'door2': {'name': 'door2', 'POS': (320, 0), 'DIM': (64, 64), 'spriteoffset': [0, 0], 'sprites': {'START:0': 'door0', 'DOOR:0': 'door0'}, 'scripts': {'START:0': ['', 'set self state DOOR', 'set self frame 0', ''], 'DOOR:0': ['', 'set self frame 0', ''], 'COLLIDE': ['', 'if UP_DOWN in inpEVENTS ', '   move self root', '   view self.frame_name root', 'endif']}, 'tangible': False}, 'door': {'name': 'door', 'POS': (320, 0), 'DIM': (64, 64), 'spriteoffset': [0, 0], 'sprites': {'START:0': 'door0', 'DOOR:0': 'door0'}, 'scripts': {'START:0': ['', 'set self state DOOR', ''], 'DOOR:0': ['', 'set self frame 0', ''], 'COLLIDE': ['', 'if UP_DOWN in inpEVENTS ', '   move self world2', '   view self.frame_name world2', 'endif']}, 'tangible': False}, 'plat3': {'name': 'plat3', 'POS': (-128, -192), 'DIM': (64, 320), 'spriteoffset': [0, 0], 'sprites': {'PLATFORM00': 'platform000', 'PLATFORM01': 'platform001', 'PLATFORM02': 'platform002', 'PLATFORM10': 'platform010', 'PLATFORM11': 'platform011', 'PLATFORM12': 'platform012', 'PLATFORM20': 'platform020', 'PLATFORM21': 'platform021', 'PLATFORM22': 'platform022', 'TOUCHED00': 'touched000', 'TOUCHED01': 'touched001', 'TOUCHED02': 'touched002', 'TOUCHED10': 'touched010', 'TOUCHED11': 'touched011', 'TOUCHED12': 'touched012', 'TOUCHED20': 'touched020', 'TOUCHED21': 'touched021', 'TOUCHED22': 'touched022'}, 'scripts': {'START:0': ['', 'set self state PLATFORM', 'set self platform 1', 'set self frame 0', 'set self touched 0', ''], 'PLATFORM:0': ['', 'if self.touched > 0', '    set self state TOUCHED', 'endif', 'set self frame 0', ''], 'TOUCHED:0': ['', 'set self frame 0']}, 'tangible': True}, 'plat4': {'name': 'plat4', 'POS': (384, 128), 'DIM': (64, 384), 'spriteoffset': [0, 0], 'sprites': {'PLATFORM00': 'platform000', 'PLATFORM01': 'platform001', 'PLATFORM02': 'platform002', 'PLATFORM10': 'platform010', 'PLATFORM11': 'platform011', 'PLATFORM12': 'platform012', 'PLATFORM20': 'platform020', 'PLATFORM21': 'platform021', 'PLATFORM22': 'platform022', 'TOUCHED00': 'touched000', 'TOUCHED01': 'touched001', 'TOUCHED02': 'touched002', 'TOUCHED10': 'touched010', 'TOUCHED11': 'touched011', 'TOUCHED12': 'touched012', 'TOUCHED20': 'touched020', 'TOUCHED21': 'touched021', 'TOUCHED22': 'touched022'}, 'scripts': {'START:0': ['', 'set self state PLATFORM', 'set self platform 1', 'set self frame 0', 'set self touched 0', ''], 'PLATFORM:0': ['', 'if self.touched > 0', '    set self state TOUCHED', 'endif', 'set self frame 0', ''], 'TOUCHED:0': ['', 'set self frame 0']}, 'tangible': True}, 'plat5': {'name': 'plat5', 'POS': (384, 512), 'DIM': (640, 64), 'spriteoffset': [0, 0], 'sprites': {'PLATFORM00': 'platform000', 'PLATFORM01': 'platform001', 'PLATFORM02': 'platform002', 'PLATFORM10': 'platform010', 'PLATFORM11': 'platform011', 'PLATFORM12': 'platform012', 'PLATFORM20': 'platform020', 'PLATFORM21': 'platform021', 'PLATFORM22': 'platform022', 'TOUCHED00': 'touched000', 'TOUCHED01': 'touched001', 'TOUCHED02': 'touched002', 'TOUCHED10': 'touched010', 'TOUCHED11': 'touched011', 'TOUCHED12': 'touched012', 'TOUCHED20': 'touched020', 'TOUCHED21': 'touched021', 'TOUCHED22': 'touched022'}, 'scripts': {'START:0': ['', 'set self state PLATFORM', 'set self platform 1', 'set self frame 0', 'set self touched 0', ''], 'PLATFORM:0': ['', 'if self.touched > 0', '    set self state TOUCHED', 'endif', 'set self frame 0', ''], 'TOUCHED:0': ['', 'set self frame 0']}, 'tangible': True}, 'plat7': {'name': 'plat7', 'POS': (1024, 64), 'DIM': (64, 512), 'spriteoffset': [0, 0], 'sprites': {'PLATFORM00': 'platform000', 'PLATFORM01': 'platform001', 'PLATFORM02': 'platform002', 'PLATFORM10': 'platform010', 'PLATFORM11': 'platform011', 'PLATFORM12': 'platform012', 'PLATFORM20': 'platform020', 'PLATFORM21': 'platform021', 'PLATFORM22': 'platform022', 'TOUCHED00': 'touched000', 'TOUCHED01': 'touched001', 'TOUCHED02': 'touched002', 'TOUCHED10': 'touched010', 'TOUCHED11': 'touched011', 'TOUCHED12': 'touched012', 'TOUCHED20': 'touched020', 'TOUCHED21': 'touched021', 'TOUCHED22': 'touched022'}, 'scripts': {'START:0': ['', 'set self state PLATFORM', 'set self platform 1', 'set self frame 0', 'set self touched 0', ''], 'PLATFORM:0': ['', 'if self.touched > 0', '    set self state TOUCHED', 'endif', 'set self frame 0', ''], 'TOUCHED:0': ['', 'set self frame 0']}, 'tangible': True}, 'plat6': {'name': 'plat6', 'POS': (1088, 64), 'DIM': (640, 64), 'spriteoffset': [0, 0], 'sprites': {'PLATFORM00': 'platform000', 'PLATFORM01': 'platform001', 'PLATFORM02': 'platform002', 'PLATFORM10': 'platform010', 'PLATFORM11': 'platform011', 'PLATFORM12': 'platform012', 'PLATFORM20': 'platform020', 'PLATFORM21': 'platform021', 'PLATFORM22': 'platform022', 'TOUCHED00': 'touched000', 'TOUCHED01': 'touched001', 'TOUCHED02': 'touched002', 'TOUCHED10': 'touched010', 'TOUCHED11': 'touched011', 'TOUCHED12': 'touched012', 'TOUCHED20': 'touched020', 'TOUCHED21': 'touched021', 'TOUCHED22': 'touched022'}, 'scripts': {'START:0': ['', 'set self state PLATFORM', 'set self platform 1', 'set self frame 0', 'set self touched 0', ''], 'PLATFORM:0': ['', 'if self.touched > 0', '    set self state TOUCHED', 'endif', 'set self frame 0', ''], 'TOUCHED:0': ['', 'set self frame 0']}, 'tangible': True}, 'plat8': {'name': 'plat8', 'POS': (1728, -192), 'DIM': (64, 320), 'spriteoffset': [0, 0], 'sprites': {'PLATFORM00': 'platform000', 'PLATFORM01': 'platform001', 'PLATFORM02': 'platform002', 'PLATFORM10': 'platform010', 'PLATFORM11': 'platform011', 'PLATFORM12': 'platform012', 'PLATFORM20': 'platform020', 'PLATFORM21': 'platform021', 'PLATFORM22': 'platform022', 'TOUCHED00': 'touched000', 'TOUCHED01': 'touched001', 'TOUCHED02': 'touched002', 'TOUCHED10': 'touched010', 'TOUCHED11': 'touched011', 'TOUCHED12': 'touched012', 'TOUCHED20': 'touched020', 'TOUCHED21': 'touched021', 'TOUCHED22': 'touched022'}, 'scripts': {'START:0': ['', 'set self state PLATFORM', 'set self platform 1', 'set self frame 0', 'set self touched 0', ''], 'PLATFORM:0': ['', 'if self.touched > 0', '    set self state TOUCHED', 'endif', 'set self frame 0', ''], 'TOUCHED:0': ['', 'set self frame 0']}, 'tangible': True}, 'plat10': {'name': 'plat10', 'POS': (704, -256), 'DIM': (1088, 64), 'spriteoffset': [0, 0], 'sprites': {'PLATFORM00': 'platform000', 'PLATFORM01': 'platform001', 'PLATFORM02': 'platform002', 'PLATFORM10': 'platform010', 'PLATFORM11': 'platform011', 'PLATFORM12': 'platform012', 'PLATFORM20': 'platform020', 'PLATFORM21': 'platform021', 'PLATFORM22': 'platform022', 'TOUCHED00': 'touched000', 'TOUCHED01': 'touched001', 'TOUCHED02': 'touched002', 'TOUCHED10': 'touched010', 'TOUCHED11': 'touched011', 'TOUCHED12': 'touched012', 'TOUCHED20': 'touched020', 'TOUCHED21': 'touched021', 'TOUCHED22': 'touched022'}, 'scripts': {'START:0': ['', 'set self state PLATFORM', 'set self platform 1', 'set self frame 0', 'set self touched 0', ''], 'PLATFORM:0': ['', 'if self.touched > 0', '    set self state TOUCHED', 'endif', 'set self frame 0', ''], 'TOUCHED:0': ['', 'set self frame 0']}, 'tangible': True}, 'plat11': {'name': 'plat11', 'POS': (-128, -256), 'DIM': (576, 64), 'spriteoffset': [0, 0], 'sprites': {'PLATFORM00': 'platform000', 'PLATFORM01': 'platform001', 'PLATFORM02': 'platform002', 'PLATFORM10': 'platform010', 'PLATFORM11': 'platform011', 'PLATFORM12': 'platform012', 'PLATFORM20': 'platform020', 'PLATFORM21': 'platform021', 'PLATFORM22': 'platform022', 'TOUCHED00': 'touched000', 'TOUCHED01': 'touched001', 'TOUCHED02': 'touched002', 'TOUCHED10': 'touched010', 'TOUCHED11': 'touched011', 'TOUCHED12': 'touched012', 'TOUCHED20': 'touched020', 'TOUCHED21': 'touched021', 'TOUCHED22': 'touched022'}, 'scripts': {'START:0': ['', 'set self state PLATFORM', 'set self platform 1', 'set self frame 0', 'set self touched 0', ''], 'PLATFORM:0': ['', 'if self.touched > 0', '    set self state TOUCHED', 'endif', 'set self frame 0', ''], 'TOUCHED:0': ['', 'set self frame 0']}, 'tangible': True}, 'plat12': {'name': 'plat12', 'POS': (384, -512), 'DIM': (64, 256), 'spriteoffset': [0, 0], 'sprites': {'PLATFORM00': 'platform000', 'PLATFORM01': 'platform001', 'PLATFORM02': 'platform002', 'PLATFORM10': 'platform010', 'PLATFORM11': 'platform011', 'PLATFORM12': 'platform012', 'PLATFORM20': 'platform020', 'PLATFORM21': 'platform021', 'PLATFORM22': 'platform022', 'TOUCHED00': 'touched000', 'TOUCHED01': 'touched001', 'TOUCHED02': 'touched002', 'TOUCHED10': 'touched010', 'TOUCHED11': 'touched011', 'TOUCHED12': 'touched012', 'TOUCHED20': 'touched020', 'TOUCHED21': 'touched021', 'TOUCHED22': 'touched022'}, 'scripts': {'START:0': ['', 'set self state PLATFORM', 'set self platform 1', 'set self frame 0', 'set self touched 0', ''], 'PLATFORM:0': ['', 'if self.touched > 0', '    set self state TOUCHED', 'endif', 'set self frame 0', ''], 'TOUCHED:0': ['', 'set self frame 0']}, 'tangible': True}, 'plat13': {'name': 'plat13', 'POS': (384, -576), 'DIM': (1408, 64), 'spriteoffset': [0, 0], 'sprites': {'PLATFORM00': 'platform000', 'PLATFORM01': 'platform001', 'PLATFORM02': 'platform002', 'PLATFORM10': 'platform010', 'PLATFORM11': 'platform011', 'PLATFORM12': 'platform012', 'PLATFORM20': 'platform020', 'PLATFORM21': 'platform021', 'PLATFORM22': 'platform022', 'TOUCHED00': 'touched000', 'TOUCHED01': 'touched001', 'TOUCHED02': 'touched002', 'TOUCHED10': 'touched010', 'TOUCHED11': 'touched011', 'TOUCHED12': 'touched012', 'TOUCHED20': 'touched020', 'TOUCHED21': 'touched021', 'TOUCHED22': 'touched022'}, 'scripts': {'START:0': ['', 'set self state PLATFORM', 'set self platform 1', 'set self frame 0', 'set self touched 0', ''], 'PLATFORM:0': ['', 'if self.touched > 0', '    set self state TOUCHED', 'endif', 'set self frame 0', ''], 'TOUCHED:0': ['', 'set self frame 0']}, 'tangible': True}, 'plat14': {'name': 'plat14', 'POS': (1728, -512), 'DIM': (64, 256), 'spriteoffset': [0, 0], 'sprites': {'PLATFORM00': 'platform000', 'PLATFORM01': 'platform001', 'PLATFORM02': 'platform002', 'PLATFORM10': 'platform010', 'PLATFORM11': 'platform011', 'PLATFORM12': 'platform012', 'PLATFORM20': 'platform020', 'PLATFORM21': 'platform021', 'PLATFORM22': 'platform022', 'TOUCHED00': 'touched000', 'TOUCHED01': 'touched001', 'TOUCHED02': 'touched002', 'TOUCHED10': 'touched010', 'TOUCHED11': 'touched011', 'TOUCHED12': 'touched012', 'TOUCHED20': 'touched020', 'TOUCHED21': 'touched021', 'TOUCHED22': 'touched022'}, 'scripts': {'START:0': ['', 'set self state PLATFORM', 'set self platform 1', 'set self frame 0', 'set self touched 0', ''], 'PLATFORM:0': ['', 'if self.touched > 0', '    set self state TOUCHED', 'endif', 'set self frame 0', ''], 'TOUCHED:0': ['', 'set self frame 0']}, 'tangible': True}, 'plat15': {'name': 'plat15', 'POS': (800, 128), 'DIM': (32, 128), 'spriteoffset': [0, 0], 'sprites': {'PLATFORM00': 'platform000', 'PLATFORM01': 'platform001', 'PLATFORM02': 'platform002', 'PLATFORM10': 'platform010', 'PLATFORM11': 'platform011', 'PLATFORM12': 'platform012', 'PLATFORM20': 'platform020', 'PLATFORM21': 'platform021', 'PLATFORM22': 'platform022', 'TOUCHED00': 'touched000', 'TOUCHED01': 'touched001', 'TOUCHED02': 'touched002', 'TOUCHED10': 'touched010', 'TOUCHED11': 'touched011', 'TOUCHED12': 'touched012', 'TOUCHED20': 'touched020', 'TOUCHED21': 'touched021', 'TOUCHED22': 'touched022'}, 'scripts': {'START:0': ['', 'set self state PLATFORM', 'set self platform 1', 'set self frame 0', 'set self touched 0', ''], 'PLATFORM:0': ['', 'if self.touched > 0', '    set self state TOUCHED', 'endif', 'set self frame 0', ''], 'TOUCHED:0': ['', 'set self frame 0']}, 'tangible': True}, 'door3': {'name': 'door3', 'POS': (1472, -320), 'DIM': (64, 64), 'spriteoffset': [0, 0], 'sprites': {'START:0': 'door0', 'DOOR:0': 'door0'}, 'scripts': {'START:0': ['', 'set self state DOOR', ''], 'DOOR:0': ['', 'set self frame 0', ''], 'COLLIDE': ['', 'if UP_DOWN in inpEVENTS ', '   move self world2', '   view self.frame_name world2', 'endif']}, 'tangible': False}, 'door4': {'name': 'door4', 'POS': (1472, -320), 'DIM': (64, 64), 'spriteoffset': [0, 0], 'sprites': {'START:0': 'door0', 'DOOR:0': 'door0'}, 'scripts': {'START:0': ['', 'set self state DOOR', 'set self frame 0', ''], 'DOOR:0': ['', 'set self frame 0', ''], 'COLLIDE': ['', 'if UP_DOWN in inpEVENTS ', '   move self root', '   view self.frame_name root', 'endif']}, 'tangible': False}, 'plat16': {'name': 'plat16', 'POS': (1440, -256), 'DIM': (128, 32), 'spriteoffset': [0, 0], 'sprites': {'PLATFORM00': 'platform000', 'PLATFORM01': 'platform001', 'PLATFORM02': 'platform002', 'PLATFORM10': 'platform010', 'PLATFORM11': 'platform011', 'PLATFORM12': 'platform012', 'PLATFORM20': 'platform020', 'PLATFORM21': 'platform021', 'PLATFORM22': 'platform022', 'TOUCHED00': 'touched000', 'TOUCHED01': 'touched001', 'TOUCHED02': 'touched002', 'TOUCHED10': 'touched010', 'TOUCHED11': 'touched011', 'TOUCHED12': 'touched012', 'TOUCHED20': 'touched020', 'TOUCHED21': 'touched021', 'TOUCHED22': 'touched022'}, 'scripts': {'START:0': ['', 'set self state PLATFORM', 'set self platform 1', 'set self frame 0', 'set self touched 0', ''], 'PLATFORM:0': ['', 'if self.touched > 0', '    set self state TOUCHED', 'endif', 'set self frame 0', ''], 'TOUCHED:0': ['', 'set self frame 0']}, 'tangible': True}, 'plat17': {'name': 'plat17', 'POS': (1472, -192), 'DIM': (64, 416), 'spriteoffset': [0, 0], 'sprites': {'PLATFORM00': 'platform000', 'PLATFORM01': 'platform001', 'PLATFORM02': 'platform002', 'PLATFORM10': 'platform010', 'PLATFORM11': 'platform011', 'PLATFORM12': 'platform012', 'PLATFORM20': 'platform020', 'PLATFORM21': 'platform021', 'PLATFORM22': 'platform022', 'TOUCHED00': 'touched000', 'TOUCHED01': 'touched001', 'TOUCHED02': 'touched002', 'TOUCHED10': 'touched010', 'TOUCHED11': 'touched011', 'TOUCHED12': 'touched012', 'TOUCHED20': 'touched020', 'TOUCHED21': 'touched021', 'TOUCHED22': 'touched022'}, 'scripts': {'START:0': ['', 'set self state PLATFORM', 'set self platform 1', 'set self frame 0', 'set self touched 0', ''], 'PLATFORM:0': ['', 'if self.touched > 0', '    set self state TOUCHED', 'endif', 'set self frame 0', ''], 'TOUCHED:0': ['', 'set self frame 0']}, 'tangible': True}, 'plat18': {'name': 'plat18', 'POS': (1376, -224), 'DIM': (256, 32), 'spriteoffset': [0, 0], 'sprites': {'PLATFORM00': 'platform000', 'PLATFORM01': 'platform001', 'PLATFORM02': 'platform002', 'PLATFORM10': 'platform010', 'PLATFORM11': 'platform011', 'PLATFORM12': 'platform012', 'PLATFORM20': 'platform020', 'PLATFORM21': 'platform021', 'PLATFORM22': 'platform022', 'TOUCHED00': 'touched000', 'TOUCHED01': 'touched001', 'TOUCHED02': 'touched002', 'TOUCHED10': 'touched010', 'TOUCHED11': 'touched011', 'TOUCHED12': 'touched012', 'TOUCHED20': 'touched020', 'TOUCHED21': 'touched021', 'TOUCHED22': 'touched022'}, 'scripts': {'START:0': ['', 'set self state PLATFORM', 'set self platform 1', 'set self frame 0', 'set self touched 0', ''], 'PLATFORM:0': ['', 'if self.touched > 0', '    set self state TOUCHED', 'endif', 'set self frame 0', ''], 'TOUCHED:0': ['', 'set self frame 0']}, 'tangible': True}, 'door5': {'name': 'door5', 'POS': (1568, 0), 'DIM': (64, 64), 'spriteoffset': [0, 0], 'sprites': {'START:0': 'door0', 'DOOR:0': 'door0'}, 'scripts': {'START:0': ['', 'set self state DOOR', ''], 'DOOR:0': ['', 'set self frame 0', ''], 'COLLIDE': ['', 'if UP_DOWN in inpEVENTS ', '   move self world2', '   view self.frame_name world2', 'endif']}, 'tangible': False}, 'door6': {'name': 'door6', 'POS': (1568, 0), 'DIM': (64, 64), 'spriteoffset': [0, 0], 'sprites': {'START:0': 'door0', 'DOOR:0': 'door0'}, 'scripts': {'START:0': ['', 'set self state DOOR', 'set self frame 0', ''], 'DOOR:0': ['', 'set self frame 0', ''], 'COLLIDE': ['', 'if UP_DOWN in inpEVENTS ', '   move self root', '   view self.frame_name root', 'endif']}, 'tangible': False}, 'plat19': {'name': 'plat19', 'POS': (1536, 64), 'DIM': (128, 32), 'spriteoffset': [0, 0], 'sprites': {'PLATFORM00': 'platform000', 'PLATFORM01': 'platform001', 'PLATFORM02': 'platform002', 'PLATFORM10': 'platform010', 'PLATFORM11': 'platform011', 'PLATFORM12': 'platform012', 'PLATFORM20': 'platform020', 'PLATFORM21': 'platform021', 'PLATFORM22': 'platform022', 'TOUCHED00': 'touched000', 'TOUCHED01': 'touched001', 'TOUCHED02': 'touched002', 'TOUCHED10': 'touched010', 'TOUCHED11': 'touched011', 'TOUCHED12': 'touched012', 'TOUCHED20': 'touched020', 'TOUCHED21': 'touched021', 'TOUCHED22': 'touched022'}, 'scripts': {'START:0': ['', 'set self state PLATFORM', 'set self platform 1', 'set self frame 0', 'set self touched 0', ''], 'PLATFORM:0': ['', 'if self.touched > 0', '    set self state TOUCHED', 'endif', 'set self frame 0', ''], 'TOUCHED:0': ['', 'set self frame 0']}, 'tangible': True}, 'plat20': {'name': 'plat20', 'POS': (1536, -96), 'DIM': (320, 32), 'spriteoffset': [0, 0], 'sprites': {'PLATFORM00': 'platform000', 'PLATFORM01': 'platform001', 'PLATFORM02': 'platform002', 'PLATFORM10': 'platform010', 'PLATFORM11': 'platform011', 'PLATFORM12': 'platform012', 'PLATFORM20': 'platform020', 'PLATFORM21': 'platform021', 'PLATFORM22': 'platform022', 'TOUCHED00': 'touched000', 'TOUCHED01': 'touched001', 'TOUCHED02': 'touched002', 'TOUCHED10': 'touched010', 'TOUCHED11': 'touched011', 'TOUCHED12': 'touched012', 'TOUCHED20': 'touched020', 'TOUCHED21': 'touched021', 'TOUCHED22': 'touched022'}, 'scripts': {'START:0': ['', 'set self state PLATFORM', 'set self platform 1', 'set self frame 0', 'set self touched 0', ''], 'PLATFORM:0': ['', 'if self.touched > 0', '    set self state TOUCHED', 'endif', 'set self frame 0', ''], 'TOUCHED:0': ['', 'set self frame 0']}, 'tangible': True}, 'plat21': {'name': 'plat21', 'POS': (1856, -96), 'DIM': (32, 544), 'spriteoffset': [0, 0], 'sprites': {'PLATFORM00': 'platform000', 'PLATFORM01': 'platform001', 'PLATFORM02': 'platform002', 'PLATFORM10': 'platform010', 'PLATFORM11': 'platform011', 'PLATFORM12': 'platform012', 'PLATFORM20': 'platform020', 'PLATFORM21': 'platform021', 'PLATFORM22': 'platform022', 'TOUCHED00': 'touched000', 'TOUCHED01': 'touched001', 'TOUCHED02': 'touched002', 'TOUCHED10': 'touched010', 'TOUCHED11': 'touched011', 'TOUCHED12': 'touched012', 'TOUCHED20': 'touched020', 'TOUCHED21': 'touched021', 'TOUCHED22': 'touched022'}, 'scripts': {'START:0': ['', 'set self state PLATFORM', 'set self platform 1', 'set self frame 0', 'set self touched 0', ''], 'PLATFORM:0': ['', 'if self.touched > 0', '    set self state TOUCHED', 'endif', 'set self frame 0', ''], 'TOUCHED:0': ['', 'set self frame 0']}, 'tangible': True}, 'plat22': {'name': 'plat22', 'POS': (-32, 384), 'DIM': (1888, 64), 'spriteoffset': [0, 0], 'sprites': {'PLATFORM00': 'platform000', 'PLATFORM01': 'platform001', 'PLATFORM02': 'platform002', 'PLATFORM10': 'platform010', 'PLATFORM11': 'platform011', 'PLATFORM12': 'platform012', 'PLATFORM20': 'platform020', 'PLATFORM21': 'platform021', 'PLATFORM22': 'platform022', 'TOUCHED00': 'touched000', 'TOUCHED01': 'touched001', 'TOUCHED02': 'touched002', 'TOUCHED10': 'touched010', 'TOUCHED11': 'touched011', 'TOUCHED12': 'touched012', 'TOUCHED20': 'touched020', 'TOUCHED21': 'touched021', 'TOUCHED22': 'touched022'}, 'scripts': {'START:0': ['', 'set self state PLATFORM', 'set self platform 1', 'set self frame 0', 'set self touched 0', ''], 'PLATFORM:0': ['', 'if self.touched > 0', '    set self state TOUCHED', 'endif', 'set self frame 0', ''], 'TOUCHED:0': ['', 'set self frame 0']}, 'tangible': True}, 'plat23': {'name': 'plat23', 'POS': (-32, -384), 'DIM': (64, 768), 'spriteoffset': [0, 0], 'sprites': {'PLATFORM00': 'platform000', 'PLATFORM01': 'platform001', 'PLATFORM02': 'platform002', 'PLATFORM10': 'platform010', 'PLATFORM11': 'platform011', 'PLATFORM12': 'platform012', 'PLATFORM20': 'platform020', 'PLATFORM21': 'platform021', 'PLATFORM22': 'platform022', 'TOUCHED00': 'touched000', 'TOUCHED01': 'touched001', 'TOUCHED02': 'touched002', 'TOUCHED10': 'touched010', 'TOUCHED11': 'touched011', 'TOUCHED12': 'touched012', 'TOUCHED20': 'touched020', 'TOUCHED21': 'touched021', 'TOUCHED22': 'touched022'}, 'scripts': {'START:0': ['', 'set self state PLATFORM', 'set self platform 1', 'set self frame 0', 'set self touched 0', ''], 'PLATFORM:0': ['', 'if self.touched > 0', '    set self state TOUCHED', 'endif', 'set self frame 0', ''], 'TOUCHED:0': ['', 'set self frame 0']}, 'tangible': True}, 'plat25': {'name': 'plat25', 'POS': (224, -160), 'DIM': (64, 288), 'spriteoffset': [0, 0], 'sprites': {'PLATFORM00': 'platform000', 'PLATFORM01': 'platform001', 'PLATFORM02': 'platform002', 'PLATFORM10': 'platform010', 'PLATFORM11': 'platform011', 'PLATFORM12': 'platform012', 'PLATFORM20': 'platform020', 'PLATFORM21': 'platform021', 'PLATFORM22': 'platform022', 'TOUCHED00': 'touched000', 'TOUCHED01': 'touched001', 'TOUCHED02': 'touched002', 'TOUCHED10': 'touched010', 'TOUCHED11': 'touched011', 'TOUCHED12': 'touched012', 'TOUCHED20': 'touched020', 'TOUCHED21': 'touched021', 'TOUCHED22': 'touched022'}, 'scripts': {'START:0': ['', 'set self state PLATFORM', 'set self platform 1', 'set self frame 0', 'set self touched 0', ''], 'PLATFORM:0': ['', 'if self.touched > 0', '    set self state TOUCHED', 'endif', 'set self frame 0', ''], 'TOUCHED:0': ['', 'set self frame 0']}, 'tangible': True}, 'plat26': {'name': 'plat26', 'POS': (-32, -448), 'DIM': (1920, 64), 'spriteoffset': [0, 0], 'sprites': {'PLATFORM00': 'platform000', 'PLATFORM01': 'platform001', 'PLATFORM02': 'platform002', 'PLATFORM10': 'platform010', 'PLATFORM11': 'platform011', 'PLATFORM12': 'platform012', 'PLATFORM20': 'platform020', 'PLATFORM21': 'platform021', 'PLATFORM22': 'platform022', 'TOUCHED00': 'touched000', 'TOUCHED01': 'touched001', 'TOUCHED02': 'touched002', 'TOUCHED10': 'touched010', 'TOUCHED11': 'touched011', 'TOUCHED12': 'touched012', 'TOUCHED20': 'touched020', 'TOUCHED21': 'touched021', 'TOUCHED22': 'touched022'}, 'scripts': {'START:0': ['', 'set self state PLATFORM', 'set self platform 1', 'set self frame 0', 'set self touched 0', ''], 'PLATFORM:0': ['', 'if self.touched > 0', '    set self state TOUCHED', 'endif', 'set self frame 0', ''], 'TOUCHED:0': ['', 'set self frame 0']}, 'tangible': True}, 'plat27': {'name': 'plat27', 'POS': (1856, -384), 'DIM': (32, 288), 'spriteoffset': [0, 0], 'sprites': {'PLATFORM00': 'platform000', 'PLATFORM01': 'platform001', 'PLATFORM02': 'platform002', 'PLATFORM10': 'platform010', 'PLATFORM11': 'platform011', 'PLATFORM12': 'platform012', 'PLATFORM20': 'platform020', 'PLATFORM21': 'platform021', 'PLATFORM22': 'platform022', 'TOUCHED00': 'touched000', 'TOUCHED01': 'touched001', 'TOUCHED02': 'touched002', 'TOUCHED10': 'touched010', 'TOUCHED11': 'touched011', 'TOUCHED12': 'touched012', 'TOUCHED20': 'touched020', 'TOUCHED21': 'touched021', 'TOUCHED22': 'touched022'}, 'scripts': {'START:0': ['', 'set self state PLATFORM', 'set self platform 1', 'set self frame 0', 'set self touched 0', ''], 'PLATFORM:0': ['', 'if self.touched > 0', '    set self state TOUCHED', 'endif', 'set self frame 0', ''], 'TOUCHED:0': ['', 'set self frame 0']}, 'tangible': True}, 'plat29': {'name': 'plat29', 'POS': (768, -160), 'DIM': (64, 224), 'spriteoffset': [0, 0], 'sprites': {'PLATFORM00': 'platform000', 'PLATFORM01': 'platform001', 'PLATFORM02': 'platform002', 'PLATFORM10': 'platform010', 'PLATFORM11': 'platform011', 'PLATFORM12': 'platform012', 'PLATFORM20': 'platform020', 'PLATFORM21': 'platform021', 'PLATFORM22': 'platform022', 'TOUCHED00': 'touched000', 'TOUCHED01': 'touched001', 'TOUCHED02': 'touched002', 'TOUCHED10': 'touched010', 'TOUCHED11': 'touched011', 'TOUCHED12': 'touched012', 'TOUCHED20': 'touched020', 'TOUCHED21': 'touched021', 'TOUCHED22': 'touched022'}, 'scripts': {'START:0': ['', 'set self state PLATFORM', 'set self platform 1', 'set self frame 0', 'set self touched 0', ''], 'PLATFORM:0': ['', 'if self.touched > 0', '    set self state TOUCHED', 'endif', 'set self frame 0', ''], 'TOUCHED:0': ['', 'set self frame 0']}, 'tangible': True}, 'plat30': {'name': 'plat30', 'POS': (288, -160), 'DIM': (480, 64), 'spriteoffset': [0, 0], 'sprites': {'PLATFORM00': 'platform000', 'PLATFORM01': 'platform001', 'PLATFORM02': 'platform002', 'PLATFORM10': 'platform010', 'PLATFORM11': 'platform011', 'PLATFORM12': 'platform012', 'PLATFORM20': 'platform020', 'PLATFORM21': 'platform021', 'PLATFORM22': 'platform022', 'TOUCHED00': 'touched000', 'TOUCHED01': 'touched001', 'TOUCHED02': 'touched002', 'TOUCHED10': 'touched010', 'TOUCHED11': 'touched011', 'TOUCHED12': 'touched012', 'TOUCHED20': 'touched020', 'TOUCHED21': 'touched021', 'TOUCHED22': 'touched022'}, 'scripts': {'START:0': ['', 'set self state PLATFORM', 'set self platform 1', 'set self frame 0', 'set self touched 0', ''], 'PLATFORM:0': ['', 'if self.touched > 0', '    set self state TOUCHED', 'endif', 'set self frame 0', ''], 'TOUCHED:0': ['', 'set self frame 0']}, 'tangible': True}, 'plat31': {'name': 'plat31', 'POS': (1024, -160), 'DIM': (96, 32), 'spriteoffset': [0, 0], 'sprites': {'PLATFORM00': 'platform000', 'PLATFORM01': 'platform001', 'PLATFORM02': 'platform002', 'PLATFORM10': 'platform010', 'PLATFORM11': 'platform011', 'PLATFORM12': 'platform012', 'PLATFORM20': 'platform020', 'PLATFORM21': 'platform021', 'PLATFORM22': 'platform022', 'TOUCHED00': 'touched000', 'TOUCHED01': 'touched001', 'TOUCHED02': 'touched002', 'TOUCHED10': 'touched010', 'TOUCHED11': 'touched011', 'TOUCHED12': 'touched012', 'TOUCHED20': 'touched020', 'TOUCHED21': 'touched021', 'TOUCHED22': 'touched022'}, 'scripts': {'START:0': ['', 'set self state PLATFORM', 'set self platform 1', 'set self frame 0', 'set self touched 0', ''], 'PLATFORM:0': ['', 'if self.touched > 0', '    set self state TOUCHED', 'endif', 'set self frame 0', ''], 'TOUCHED:0': ['', 'set self frame 0']}, 'tangible': True}}