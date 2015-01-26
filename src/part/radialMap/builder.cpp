/***********************************************************************
* Copyright 2003-2004  Max Howell <max.howell@methylblue.com>
* Copyright 2008-2009  Martin Sandsmark <martin.sandsmark@kde.org>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License or (at your option) version 3 or any later version
* accepted by the membership of KDE e.V. (or its successor approved
* by the membership of KDE e.V.), which shall act as a proxy
* defined in Section 14 of version 3 of the license.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************/

#include "builder.h"

#include "widget.h"
#include <part/Config.h>
#include <part/fileTree.h>
#include <KFormat>

//**** REMOVE NEED FOR the +1 with MAX_RING_DEPTH uses
//**** add some angle bounds checking (possibly in Segment ctor? can I delete in a ctor?)
//**** this class is a mess

RadialMap::Builder::Builder(RadialMap::Map *m, const Folder* const d, bool fast)
        : m_map(m)
        , m_root(d)
        , m_minSize(static_cast<unsigned int>((d->size() * 3) / (PI * m->height() - m->MAP_2MARGIN)))
        , m_depth(&m->m_visibleDepth)
{
    m_signature = new Chain<Segment> [*m_depth + 1];

    if (!fast)//|| *m_depth == 0) //depth 0 is special case usability-wise //**** WHY?!
    {
        //determine depth rather than use old one
        findVisibleDepth(d); //sets m_depth
    }

    m_map->setRingBreadth();
    setLimits(m_map->m_ringBreadth);
    build(d);

    m_map->m_signature = m_signature;

    delete [] m_limits;
}


void
RadialMap::Builder::findVisibleDepth(const Folder* const dir, const unsigned int depth)
{
    //**** because I don't use the same minimumSize criteria as in the visual function
    //     this can lead to incorrect visual representation
    //**** BUT, you can't set those limits until you know m_depth!

    //**** also this function doesn't check to see if anything is actually visible
    //     it just assumes that when it reaches a new level everything in it is visible
    //     automatically. This isn't right especially as there might be no files in the
    //     dir provided to this function!

    static uint stopDepth = 0;

    if (dir == m_root)
    {
        stopDepth = *m_depth;
        *m_depth = 0;
    }

    if (*m_depth < depth) *m_depth = depth;
    if (*m_depth >= stopDepth) return;

    for (ConstIterator<File> it = dir->constIterator(); it != dir->end(); ++it)
        if ((*it)->isFolder() && (*it)->size() > m_minSize)
            findVisibleDepth((Folder *)*it, depth + 1); //if no files greater than min size the depth is still recorded
}

void
RadialMap::Builder::setLimits(const uint &b) //b = breadth?
{
    double size3 = m_root->size() * 3;
    double pi2B   = PI * 2 * b;

    m_limits = new uint [*m_depth + 1]; //FIXME delete!

    for (unsigned int d = 0; d <= *m_depth; ++d)
        m_limits[d] = (uint)(size3 / (double)(pi2B * (d + 1))); //min is angle that gives 3px outer diameter for that depth
}


//**** segments currently overlap at edges (i.e. end of first is start of next)
bool
RadialMap::Builder::build(const Folder* const dir, const unsigned int depth, unsigned int a_start, const unsigned int a_end)
{
    //first iteration: dir == m_root

    if (dir->children() == 0) //we do fileCount rather than size to avoid chance of divide by zero later
        return false;

    uint hiddenSize = 0, hiddenFileCount = 0;

    for (ConstIterator<File> it = dir->constIterator(); it != dir->end(); ++it)
    {
        if ((*it)->size() > m_limits[depth])
        {
            unsigned int a_len = (unsigned int)(5760 * ((double)(*it)->size() / (double)m_root->size()));

            Segment *s = new Segment(*it, a_start, a_len);

            (m_signature + depth)->append(s);

            if ((*it)->isFolder())
            {
                if (depth != *m_depth)
                {
                    //recurse
                    s->m_hasHiddenChildren = build((Folder*)*it, depth + 1, a_start, a_start + a_len);
                }
                else s->m_hasHiddenChildren = true;
            }

            a_start += a_len; //**** should we add 1?

        } else {

            hiddenSize += (*it)->size();

            if ((*it)->isFolder()) //**** considered virtual, but dir wouldn't count itself!
                hiddenFileCount += static_cast<const Folder*>(*it)->children(); //need to add one to count the dir as well

            ++hiddenFileCount;
        }
    }

    if (hiddenFileCount == dir->children() && !Config::showSmallFiles)
        return true;

    else if ((Config::showSmallFiles && hiddenSize > m_limits[depth]) || (depth == 0 && (hiddenSize > dir->size()/8)) /*|| > size() * 0.75*/)
    {
        //append a segment for unrepresented space - a "fake" segment

        const QString s = QObject::tr("%n file(s), with an average size of %1",
                                      "description of a segment of several smaller files",
                                      hiddenFileCount).arg(KFormat().formatByteSize(hiddenSize/hiddenFileCount));

        (m_signature + depth)->append(new Segment(new File(s.toUtf8(), hiddenSize), a_start, a_end - a_start, true));
    }

    return false;
}
