import React, { useEffect, useState } from 'react';
import { HelpCircle, ThumbsUp, ThumbsDown } from 'lucide-react';

interface TableOfContentsProps {
  headings: { id: string; text: string; level: number }[];
  onNavigateToContributing?: () => void;
}

export const TableOfContents: React.FC<TableOfContentsProps> = ({
  headings,
}) => {
  const [activeHeadingId, setActiveHeadingId] = useState<string>('');
  const [feedback, setFeedback] = useState<'up' | 'down' | null>(null);

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
      { rootMargin: '-70px 0% -60% 0%' }
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
      const topOffset = 75;
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
    <aside className="claude-toc">
      <div className="toc-sticky-container">
        <h4 className="toc-section-heading">ON THIS PAGE</h4>
        {headings.length === 0 ? (
          <p className="toc-empty">Overview</p>
        ) : (
          <ul className="toc-items-list">
            {headings.map((h) => {
              const isActive = activeHeadingId === h.id;
              return (
                <li
                  key={h.id}
                  className={`toc-item level-${h.level} ${isActive ? 'active' : ''}`}
                >
                  <button
                    onClick={() => scrollToHeading(h.id)}
                    className="toc-link-btn"
                  >
                    {h.text}
                  </button>
                </li>
              );
            })}
          </ul>
        )}

        {/* Was this helpful card matching Screenshot 2 */}
        <div className="was-this-helpful-card">
          <div className="helpful-title">
            <HelpCircle size={14} className="helpful-icon" />
            <span>Was this helpful?</span>
          </div>
          <div className="helpful-actions">
            <button
              onClick={() => setFeedback(feedback === 'up' ? null : 'up')}
              className={`feedback-btn ${feedback === 'up' ? 'selected' : ''}`}
              title="Yes, helpful"
              aria-label="Yes, this was helpful"
            >
              <ThumbsUp size={14} />
            </button>
            <button
              onClick={() => setFeedback(feedback === 'down' ? null : 'down')}
              className={`feedback-btn ${feedback === 'down' ? 'selected' : ''}`}
              title="No, not helpful"
              aria-label="No, this was not helpful"
            >
              <ThumbsDown size={14} />
            </button>
          </div>
        </div>
      </div>
    </aside>
  );
};
