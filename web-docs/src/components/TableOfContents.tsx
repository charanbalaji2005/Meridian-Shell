import React, { useEffect, useState } from 'react';
import { ChevronRight, Activity } from 'lucide-react';

interface TableOfContentsProps {
  headings: { id: string; text: string; level: number }[];
  onNavigateToContributing: () => void;
}

export const TableOfContents: React.FC<TableOfContentsProps> = ({
  headings,
  onNavigateToContributing,
}) => {
  const [activeHeadingId, setActiveHeadingId] = useState<string>('');

  useEffect(() => {
    if (headings.length > 0) {
      setActiveHeadingId(headings[0].id);
    }

    const observer = new IntersectionObserver(
      (entries) => {
        entries.forEach((entry) => {
          if (entry.isIntersecting) {
            setActiveHeadingId(entry.target.id);
          }
        });
      },
      { rootMargin: '-80px 0% -60% 0%' }
    );

    headings.forEach((h) => {
      const el = document.getElementById(h.id);
      if (el) observer.observe(el);
    });

    return () => observer.disconnect();
  }, [headings]);

  const scrollToHeading = (id: string) => {
    const el = document.getElementById(id);
    if (el) {
      const topOffset = 80;
      const elementPosition = el.getBoundingClientRect().top;
      const offsetPosition = elementPosition + window.pageYOffset - topOffset;

      window.scrollTo({
        top: offsetPosition,
        behavior: 'smooth',
      });
      setActiveHeadingId(id);
    }
  };

  return (
    <aside className="meridian-toc">
      <div className="toc-inner">
        <h4 className="toc-title">Contents</h4>
        {headings.length === 0 ? (
          <p className="toc-empty">Overview</p>
        ) : (
          <ul className="toc-list">
            {headings.map((h) => {
              const isActive = activeHeadingId === h.id;
              return (
                <li
                  key={h.id}
                  className={`toc-item level-${h.level} ${isActive ? 'active' : ''}`}
                >
                  <button
                    onClick={() => scrollToHeading(h.id)}
                    className="toc-link"
                  >
                    {h.text}
                  </button>
                </li>
              );
            })}
          </ul>
        )}

        {/* Project Status Panel matching specification */}
        <div className="toc-status-panel">
          <div className="status-panel-header">
            <Activity size={14} className="status-panel-icon" />
            <span className="status-panel-title">Project Status</span>
          </div>
          
          <div className="status-rows-list">
            <div className="status-row">
              <span className="status-row-name">PTY / Shell</span>
              <span className="status-dot dot-green">● Implemented</span>
            </div>
            <div className="status-row">
              <span className="status-row-name">VT / Screen</span>
              <span className="status-dot dot-green">● Implemented</span>
            </div>
            <div className="status-row">
              <span className="status-row-name">GUI</span>
              <span className="status-dot dot-amber">● Development</span>
            </div>
            <div className="status-row">
              <span className="status-row-name">GPU Graphics</span>
              <span className="status-dot dot-amber">● Development</span>
            </div>
            <div className="status-row">
              <span className="status-row-name">AI</span>
              <span className="status-dot dot-purple">● Experimental</span>
            </div>
            <div className="status-row">
              <span className="status-row-name">Windows</span>
              <span className="status-dot dot-amber">● Development</span>
            </div>
            <div className="status-row">
              <span className="status-row-name">macOS</span>
              <span className="status-dot dot-amber">● Development</span>
            </div>
          </div>
        </div>

        {/* Contributing Link */}
        <div className="toc-contribute-box">
          <button onClick={onNavigateToContributing} className="toc-contribute-btn">
            <span>Want to help? Contribute</span>
            <ChevronRight size={14} />
          </button>
        </div>
      </div>
    </aside>
  );
};
