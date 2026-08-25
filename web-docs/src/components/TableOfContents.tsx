import React, { useEffect, useState } from 'react';
import { ChevronRight, HeartHandshake } from 'lucide-react';

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
    <aside className="fedora-toc">
      <div className="toc-inner">
        <h4 className="toc-title">Contents</h4>
        {headings.length === 0 ? (
          <p className="toc-empty">No subheadings</p>
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

        <div className="toc-contribute-card">
          <div className="contribute-card-content">
            <HeartHandshake size={18} className="contribute-icon" />
            <div className="contribute-text">
              <span className="contribute-heading">Want to help?</span>
              <button
                onClick={onNavigateToContributing}
                className="contribute-cta-btn"
              >
                <span>Learn how to contribute</span>
                <ChevronRight size={14} />
              </button>
            </div>
          </div>
        </div>
      </div>
    </aside>
  );
};
