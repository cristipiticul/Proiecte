global baseVertex = 0 
global baseTVertex = 0 
global baseNVertex = 0 

function toDec f = ( 
local i = f as integer 
return i 
) 

function outputHeader =( 
format "\n" 
format "Copy and paste the following lines into a\n" 
format "text editor, and save them as a .obj file.\n" 
format "\n" 
format "# -----------------\n" 
format "# Start of obj file\n" 
flushStringStream 
) 

function outputFooter =( 
format "# end of obj file\n" 
format "# ---------------\n" 
format "\n" 
flushStringStream 
) 

function outputNode node = ( 
format "o %\n" node.name 

local m = snapshotAsMesh node 
for v = 1 to m.numVerts do ( 
local vert = m.verts[v] 
format "v % % %\n" vert.pos.x vert.pos.y vert.pos.z 
flushStringStream 
) 
format "\n" 

if m.numtverts > 0 do ( 
-- Texture coordinates 

for vt = 1 to m.numtverts do ( 
local tvert = getTVert m vt 
format "vt % % %\n" tvert.x tvert.y tvert.z 
) 
format "\n" 

-- Also print out the normals, because some importers 
-- can't handle faces with textures but not normals 

for f =1 to m.numFaces do ( 
local vn = getFaceNormal m f 
format "vn % % %\n" vn.x vn.y vn.z 
flushStringStream 
) 
format "\n" 
) 

local mv = m.verts 
for f =1 to m.numFaces do ( 
local vface =getface m f 
local x = baseVertex + toDec(vface.x) 
local y = baseVertex + toDec(vface.y) 
local z = baseVertex + toDec(vface.z) 

if m.numtverts > 0 then 
( 
local tvface = gettvface m f 
local tx = baseTVertex + toDec(tvface.x) 
local ty = baseTVertex + toDec(tvface.y) 
local tz = baseTVertex + toDec(tvface.z) 
local n = baseNVertex + f 
format "f %/%/% %/%/% %/%/%\n" x tx n y ty n z tz n 
flushStringStream 
) 
else ( 
format "f % % %\n" x y z 
) 
) 
format "\n" 
baseVertex = baseVertex + m.numVerts 
baseTVertex = baseTVertex + m.numtverts 
baseNVertex = baseNVertex + m.numFaces 
) 

function PrintObj = ( 
outputHeader() 
for c in rootNode.children do outputNode c 
outputFooter() 
) 

PrintObj()